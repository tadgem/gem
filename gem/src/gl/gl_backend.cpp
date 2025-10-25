#include "gem/gl/gl_backend.h"
#include "gem/dbg_memory.h"
#include "gem/gl/gl_dbg.h"
#include "gem/input.h"
#include "gem/profile.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "spdlog/spdlog.h"

#ifdef __WIN32__
#include "windows.h"
#endif

namespace gem {

// todo: rework this to allow rendering backend to init
void GLBackend::Init(BackendInit &init_props) {
  ZoneScoped;
#ifdef GEM_ENABLE_MEMORY_TRACKING
  DebugMemoryTracker::s_instance = new DebugMemoryTracker();
#endif

#ifdef __WIN32__
  SetProcessDPIAware();
#endif

  // Setup SDL
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
    spdlog::error("gl_backend : failed to initialize SDL : {}",  std::string(SDL_GetError()));
    return;
  }
  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  const char *glsl_version = "#version 150";
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS,
      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                        SDL_WINDOW_HIGH_PIXEL_DENSITY);
  window =
      SDL_CreateWindow("GEM Engine", init_props.window_resolution.x,
                       init_props.window_resolution.y, window_flags);
  if (window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return;
  }

  m_sdl_gl_context = new SDL_GLContext(SDL_GL_CreateContext(window));
  SDL_GL_MakeCurrent(window, *m_sdl_gl_context);
  glewExperimental = true;

  if (glewInit() != GLEW_OK) {
    printf("GLEW init failed!");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
  }

  SDL_GL_SetSwapInterval(init_props.enable_vsync); // Enable vsync

  glEnable(GL_DEPTH_TEST);
#ifdef __DEBUG__
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(glDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                        GL_TRUE);
#endif

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  imgui_io = &ImGui::GetIO();
  imgui_io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  imgui_io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  imgui_io->ConfigFlags |=
      ImGuiConfigFlags_DockingEnable; // Enable imgui window docking

  // Setup Dear ImGui style
  SetGemImGuiStyle();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(window, *m_sdl_gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  now_counter_ = SDL_GetPerformanceCounter();
  last_counter_ = 0;

  InitBuiltInAssets(this);
  InitImGuiFileDialogImpl();
}

void GLBackend::ProcessEvents() {
  ZoneScoped;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      quit = true;
    }
//    if (event.type == SDL_EventType::Window_E &&
//        event.window.event == SDL_WINDOWEVENT_CLOSE &&
//        event.window.windowID == SDL_GetWindowID(m_window)) {
//      m_quit = true;
//    }

    HandleInputEvents(event);
  }
  if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
    SDL_Delay(10);
  }
}

void GLBackend::PreFrame() {
  ZoneScoped;
  static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
  last_counter_ = now_counter_;
  now_counter_ = SDL_GetPerformanceCounter();

  frametime_ =
      static_cast<float>((now_counter_ - last_counter_) /
                         static_cast<float>(SDL_GetPerformanceFrequency()));

  glViewport(0, 0, (int)imgui_io->DisplaySize.x,
             (int)imgui_io->DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  float mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);
  Input::UpdateMousePosition(GetWindowDimensions(), glm::vec2(mouse_x, mouse_y));

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void GLBackend::PostFrame() {
  ZoneScoped;
  {
    GEM_GPU_MARKER("ImGui");
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
}

void GLBackend::ShutDown() {
  ZoneScoped;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  if (m_sdl_gl_context != nullptr) {
    SDL_GL_DestroyContext(*m_sdl_gl_context);
  }
  SDL_DestroyWindow(window);
  delete m_sdl_gl_context;
  SDL_Quit();
}

void GLBackend::HandleInputEvents(SDL_Event &input_event) {
  ZoneScoped;
  Input::UpdateLastFrame();

  if (input_event.type == SDL_EVENT_KEY_DOWN) {
    SDL_KeyboardEvent keyEvent = input_event.key;
    SDL_Keycode keySym = keyEvent.key;
    KeyboardKey key = Input::GetKeySDL(keySym);
    Input::UpdateKey(key, true);
    return;
  }

  if (input_event.type == SDL_EVENT_KEY_UP) {
    SDL_KeyboardEvent keyEvent = input_event.key;
    SDL_Keycode keySym = keyEvent.key;
    KeyboardKey key = Input::GetKeySDL(keySym);
    Input::UpdateKey(key, false);
    return;
  }

  // Mouse
  if (input_event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
    SDL_MouseButtonEvent buttonEvent = input_event.button;
    MouseButton button =
        buttonEvent.button == 3 ? MouseButton::right : MouseButton::left;
    Input::UpdateMouseButton(button, false);
    return;
  }
  if (input_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    SDL_MouseButtonEvent buttonEvent = input_event.button;
    MouseButton button =
        buttonEvent.button == 3 ? MouseButton::right : MouseButton::left;
    Input::UpdateMouseButton(button, true);
    return;
  }

  if (input_event.type == SDL_EVENT_GAMEPAD_ADDED) {
    SDL_OpenGamepad(input_event.gdevice.which);
  }

//  if (input_event.type == SDL_CONTROLLERDEVICEREMOVED) {
//    SDL_ControllerDeviceEvent d = input_event.cdevice;
//    // SDL_GameControllerClose(d.which);
//  }

  if (input_event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
    SDL_GamepadAxisEvent axisEvent = input_event.gaxis;
    int index = axisEvent.which;
    auto axis = (SDL_GamepadAxis)axisEvent.axis;
    float value = (float)axisEvent.value / 32767.0f;

    if (axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER ||
        axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
      Input::UpdateGamepadTrigger(index, Input::GetTriggerSDL(axis),
                                    value);
    }
    if (axis == SDL_GAMEPAD_AXIS_LEFTX ||
        axis == SDL_GAMEPAD_AXIS_RIGHTX) {
      glm::vec2 current =
          Input::GetGamepadStick(index, Input::GetStickSDL(axis));
      current.x = value;
      Input::UpdateGamepadStick(index, Input::GetStickSDL(axis),
                                  current);
    }
    if (axis == SDL_GAMEPAD_AXIS_LEFTY ||
        axis == SDL_GAMEPAD_AXIS_RIGHTY) {
      glm::vec2 current =
          Input::GetGamepadStick(index, Input::GetStickSDL(axis));
      current.y = value;
      Input::UpdateGamepadStick(index, Input::GetStickSDL(axis),
                                  current);
    }
  }

  if (input_event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
    SDL_GamepadButtonEvent buttonEvent = input_event.gbutton;

    Input::UpdateGamepadButton(buttonEvent.which,
                                 Input::GetButtonSDL(buttonEvent.button),
                                 true);
    return;
  }

  if (input_event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
    SDL_GamepadButtonEvent buttonEvent = input_event.gbutton;

    Input::UpdateGamepadButton(buttonEvent.which,
                                 Input::GetButtonSDL(buttonEvent.button),
                                 false);
    return;
  }
}

glm::vec2 GLBackend::GetWindowDimensions() {
  ZoneScoped;
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  return glm::vec2{static_cast<float>(w), static_cast<float>(h)};
}
} // namespace gem