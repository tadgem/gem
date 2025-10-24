#pragma once
#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#undef main
#include "gem/dbg_memory.h"
#include "glm.hpp"
#include "imgui.h"

namespace gem {

class AMesh;

enum class BackendAPI { open_gl, sdl };

void SetGemImGuiStyle();


void InitImGuiFileDialogImpl();

struct BackendInit {
  glm::vec2 window_resolution;
  bool enable_vsync;

  GEM_IMPL_ALLOC(BackendInit)
};

class GPUBackend {
protected:
  float p_frametime = 0.0f;
  uint64_t m_now_counter, m_last_counter;

private:
  inline static BackendAPI s_selected_backend_api;
  inline static GPUBackend *s_selected_backend = nullptr;

public:
  virtual void Init(BackendInit &init_props) = 0;
  virtual void ProcessEvents() = 0;
  virtual void PreFrame() = 0;
  virtual void PostFrame() = 0;
  virtual void ShutDown() = 0;
  virtual void HandleInputEvents(SDL_Event &input_event) = 0;
  virtual glm::vec2 GetWindowDimensions() = 0;
  virtual const BackendAPI GetAPI() = 0;

  [[nodiscard]] float GetFrameTime() const { return p_frametime; }

  SDL_Window *m_window = nullptr;
  bool m_quit = false;
  ImGuiIO *m_imgui_io = nullptr;

  static GPUBackend *Selected() { return s_selected_backend; }
  static BackendAPI GetBackendAPI() { return s_selected_backend_api; }

  template <typename _Backend, typename... Args>
  static void InitBackend(BackendInit &props, Args &&...args) {
    static_assert(std::is_base_of<GPUBackend, _Backend>());
    auto *backend = new _Backend(std::forward<Args>(args)...);
    backend->Init(props);
    s_selected_backend = static_cast<GPUBackend *>(backend);
    s_selected_backend_api = s_selected_backend->GetAPI();
  }
};


} // namespace gem
void InitBuiltInAssets(gem::GPUBackend* backend);
