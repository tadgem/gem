#define GLM_ENABLE_EXPERIMENTAL

#include "ImFileDialog.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include <iostream>

#include "gem/backend.h"
#include "gem/dbg_memory.h"
#include "gem/engine.h"
#include "gem/funnelsans.ttf.h"
#include "gem/input.h"
#include "gem/mesh.h"
#include "gem/open_gl_dbg.h"
#include "gem/profile.h"
#include "gem/shape.h"
#include "gem/texture.h"

#ifdef __WIN32__
#include "windows.h"
#endif

#undef main
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

static SDL_GLContext *s_sdl_gl_context = nullptr;
static Uint64 s_now_counter;
static Uint64 s_last_counter;

GLenum glCheckError_(const char *file, int line) {
  ZoneScoped;
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    std::string error;
    switch (errorCode) {
    case GL_INVALID_ENUM:
      error = "INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      error = "INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      error = "INVALID_OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      error = "STACK_OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      error = "STACK_UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      error = "OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error = "INVALID_FRAMEBUFFER_OPERATION";
      break;
    }
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
  }
  return errorCode;
}

static void glDebugOutput(GLenum source, GLenum type, unsigned int id,
                          GLenum severity, GLsizei length, const char *message,
                          const void *userParam) {
  ZoneScoped;
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return; // GPU Markers count as notifications

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source) {
  case GL_DEBUG_SOURCE_API:
    std::cout << "Source: API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    std::cout << "Source: Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    std::cout << "Source: Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    std::cout << "Source: Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    std::cout << "Source: Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    std::cout << "Source: Other";
    break;
  }
  std::cout << std::endl;

  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    std::cout << "Type: Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    std::cout << "Type: Deprecated Behaviour";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    std::cout << "Type: Undefined Behaviour";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    std::cout << "Type: Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    std::cout << "Type: Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    std::cout << "Type: Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    std::cout << "Type: Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    std::cout << "Type: Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    std::cout << "Type: Other";
    break;
  }
  std::cout << std::endl;

  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    std::cout << "Severity: high";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    std::cout << "Severity: medium";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    std::cout << "Severity: low";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    std::cout << "Severity: notification";
    break;
  }
  std::cout << std::endl;

  if (userParam != nullptr) {
    std::cout << "user data found" << length;
  }
  std::cout << std::endl;
}

namespace gem {
void set_imgui_style() {
  ZoneScoped;
  ImFontConfig font_config = ImFontConfig();
  font_config.FontDataOwnedByAtlas = false;
  ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void *)&funnel_sans_ttf_bin[0],
                                             FUNNELSANS_TTF_SIZE, 18.0f,
                                             &font_config);
  ImVec4 *colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
  colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
  colors[ImGuiCol_Button] = ImVec4(0.29f, 0.29f, 0.29f, 0.62f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 0.29f, 0.29f, 0.72f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.29f, 0.29f, 0.23f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
  colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
  colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
  colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);
  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowPadding = ImVec2(8.00f, 8.00f);
  style.FramePadding = ImVec2(5.00f, 2.00f);
  style.CellPadding = ImVec2(6.00f, 6.00f);
  style.ItemSpacing = ImVec2(6.00f, 6.00f);
  style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
  style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
  style.IndentSpacing = 25;
  style.ScrollbarSize = 15;
  style.GrabMinSize = 10;
  style.WindowBorderSize = 2;
  style.ChildBorderSize = 2;
  style.PopupBorderSize = 2;
  style.FrameBorderSize = 1;
  style.TabBorderSize = 1;
  style.WindowRounding = 7;
  style.ChildRounding = 4;
  style.FrameRounding = 1;
  style.PopupRounding = 4;
  style.ScrollbarRounding = 9;
  style.GrabRounding = 3;
  style.LogSliderDeadzone = 4;
  style.TabRounding = 4;
}

void init_built_in_assets() {
  ZoneScoped;
  std::vector<unsigned int> black_data = {0};
  std::vector<unsigned int> white_data = {UINT32_MAX};
  texture::white = new texture(
      texture::from_data(white_data.data(), white_data.size(), 1, 1, 1, 4));
  texture::black = new texture(
      texture::from_data(black_data.data(), white_data.size(), 1, 1, 1, 4));

  shapes::init_built_in_assets(engine::assets);

  //mesh::s_cube = mesh {shapes::s_cube, 36, gem::aabb{{0,0,0}, {1,1,1}}};
}

void init_imgui_file_dialog() {
  ZoneScoped;
  ifd::FileDialog::Instance().CreateTexture = [](uint8_t *data, int w, int h,
                                                 char fmt) -> void * {
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return (void *)tex;
  };

  ifd::FileDialog::Instance().DeleteTexture = [](void *tex) {
    GLuint texID = (GLuint)tex;
    glDeleteTextures(1, &texID);
  };
}

// todo: rework this to allow rendering backend to init
void gl_backend::init(backend_init &init_props) {
  ZoneScoped;
#ifdef ENABLE_MEMORY_TRACKING
  debug_memory_tracker::s_instance = new debug_memory_tracker();
#endif

#ifdef __WIN32__
  SetProcessDPIAware();
#endif

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
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
                        SDL_WINDOW_ALLOW_HIGHDPI);
  s_window =
      SDL_CreateWindow("graphics engine", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, init_props.window_resolution.x,
                       init_props.window_resolution.y, window_flags);
  if (s_window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return;
  }

  s_sdl_gl_context = new SDL_GLContext(SDL_GL_CreateContext(s_window));
  SDL_GL_MakeCurrent(s_window, *s_sdl_gl_context);
  glewExperimental = true;

  if (glewInit() != GLEW_OK) {
    printf("GLEW init failed!");
    SDL_DestroyWindow(s_window);
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
  s_imgui_io = &ImGui::GetIO();
  s_imgui_io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  s_imgui_io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  s_imgui_io->ConfigFlags |=
      ImGuiConfigFlags_DockingEnable; // Enable imgui window docking

  // Setup Dear ImGui style
  set_imgui_style();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(s_window, *s_sdl_gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  s_now_counter = SDL_GetPerformanceCounter();
  s_last_counter = 0;

  init_built_in_assets();
  init_imgui_file_dialog();
}

void gl_backend::process_sdl_event() {
  ZoneScoped;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      s_quit = true;
    }
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(s_window)) {
      s_quit = true;
    }

    engine_handle_input_events(event);
  }
  if (SDL_GetWindowFlags(s_window) & SDL_WINDOW_MINIMIZED) {
    SDL_Delay(10);
  }
}

void gl_backend::engine_pre_frame() {
  ZoneScoped;
  static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
  s_last_counter = s_now_counter;
  s_now_counter = SDL_GetPerformanceCounter();

  s_frametime =         static_cast<float>((s_now_counter - s_last_counter) /
                        static_cast<float>(SDL_GetPerformanceFrequency()));

  glViewport(0, 0, (int)s_imgui_io->DisplaySize.x,
             (int)s_imgui_io->DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  int mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);
  input::update_mouse_position(get_window_dim(), glm::vec2(mouse_x, mouse_y));

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void gl_backend::engine_post_frame() {
  ZoneScoped;
  {
    GPU_MARKER("ImGui");
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(s_window);
  }
}

void gl_backend::engine_shut_down() {
  ZoneScoped;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  if(s_sdl_gl_context != nullptr)
  {
    SDL_GL_DeleteContext(*s_sdl_gl_context);
  }
  SDL_DestroyWindow(s_window);
  delete s_sdl_gl_context;
  SDL_Quit();
}

float gl_backend::get_frame_time() {
  ZoneScoped;
  return s_frametime;
}

void gl_backend::engine_handle_input_events(SDL_Event &input_event) {
  ZoneScoped;
  input::update_last_frame();

  if (input_event.type == SDL_KEYDOWN) {
    SDL_KeyboardEvent keyEvent = input_event.key;
    SDL_Keysym keySym = keyEvent.keysym;
    keyboard_key key = input::get_key_from_sdl(keySym.sym);
    input::update_keyboard_key(key, true);
    return;
  }

  if (input_event.type == SDL_KEYUP) {
    SDL_KeyboardEvent keyEvent = input_event.key;
    SDL_Keysym keySym = keyEvent.keysym;
    keyboard_key key = input::get_key_from_sdl(keySym.sym);
    input::update_keyboard_key(key, false);
    return;
  }

  // Mouse
  if (input_event.type == SDL_MOUSEBUTTONUP) {
    SDL_MouseButtonEvent buttonEvent = input_event.button;
    mouse_button button =
        buttonEvent.button == 3 ? mouse_button::right : mouse_button::left;
    input::update_mouse_button(button, false);
    return;
  }
  if (input_event.type == SDL_MOUSEBUTTONDOWN) {
    SDL_MouseButtonEvent buttonEvent = input_event.button;
    mouse_button button =
        buttonEvent.button == 3 ? mouse_button::right : mouse_button::left;
    input::update_mouse_button(button, true);
    return;
  }

  if (input_event.type == SDL_CONTROLLERDEVICEADDED) {
    SDL_ControllerDeviceEvent d = input_event.cdevice;
    SDL_GameControllerOpen(d.which);
  }

  if (input_event.type == SDL_CONTROLLERDEVICEREMOVED) {
    SDL_ControllerDeviceEvent d = input_event.cdevice;
    // SDL_GameControllerClose(d.which);
  }

  if (input_event.type == SDL_CONTROLLERAXISMOTION) {
    SDL_ControllerAxisEvent axisEvent = input_event.caxis;
    int index = axisEvent.which;
    auto axis = (SDL_GameControllerAxis)axisEvent.axis;
    float value = (float)axisEvent.value / 32767.0f;

    if (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT ||
        axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
      input::update_gamepad_trigger(index, input::get_trigger_from_sdl(axis),
                                    value);
    }
    if (axis == SDL_CONTROLLER_AXIS_LEFTX ||
        axis == SDL_CONTROLLER_AXIS_RIGHTX) {
      glm::vec2 current =
          input::get_gamepad_stick(index, input::get_stick_from_sdl(axis));
      current.x = value;
      input::update_gamepad_stick(index, input::get_stick_from_sdl(axis),
                                  current);
    }
    if (axis == SDL_CONTROLLER_AXIS_LEFTY ||
        axis == SDL_CONTROLLER_AXIS_RIGHTY) {
      glm::vec2 current =
          input::get_gamepad_stick(index, input::get_stick_from_sdl(axis));
      current.y = value;
      input::update_gamepad_stick(index, input::get_stick_from_sdl(axis),
                                  current);
    }
  }

  if (input_event.type == SDL_CONTROLLERBUTTONDOWN) {
    SDL_ControllerButtonEvent buttonEvent = input_event.cbutton;

    input::update_gamepad_button(buttonEvent.which,
                                 input::get_button_from_sdl(buttonEvent.button),
                                 true);
    return;
  }

  if (input_event.type == SDL_CONTROLLERBUTTONUP) {
    SDL_ControllerButtonEvent buttonEvent = input_event.cbutton;

    input::update_gamepad_button(buttonEvent.which,
                                 input::get_button_from_sdl(buttonEvent.button),
                                 false);
    return;
  }
}

glm::vec2 gl_backend::get_window_dim() {
  ZoneScoped;
  int w, h;
  SDL_GetWindowSize(s_window, &w, &h);
  return glm::vec2{static_cast<float>(w), static_cast<float>(h)};
}
} // namespace gem