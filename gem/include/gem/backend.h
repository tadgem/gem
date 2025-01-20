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

void set_imgui_style();


void init_imgui_file_dialog();

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
  virtual void init(BackendInit &init_props) = 0;
  virtual void process_sdl_event() = 0;
  virtual void engine_pre_frame() = 0;
  virtual void engine_post_frame() = 0;
  virtual void engine_shut_down() = 0;
  virtual void engine_handle_input_events(SDL_Event &input_event) = 0;
  virtual glm::vec2 get_window_dim() = 0;
  virtual const BackendAPI get_backend_api_enum() = 0;

  [[nodiscard]] float get_frame_time() const { return p_frametime; }

  SDL_Window *m_window = nullptr;
  bool m_quit = false;
  ImGuiIO *m_imgui_io = nullptr;

  static GPUBackend *selected() { return s_selected_backend; }
  static BackendAPI get_backend_api() { return s_selected_backend_api; }

  template <typename _Backend, typename... Args>
  static void init_backend(BackendInit &props, Args &&...args) {
    static_assert(std::is_base_of<GPUBackend, _Backend>());
    auto *backend = new _Backend(std::forward<Args>(args)...);
    backend->init(props);
    s_selected_backend = static_cast<GPUBackend *>(backend);
    s_selected_backend_api = s_selected_backend->get_backend_api_enum();
  }
};


} // namespace gem
void init_built_in_assets(gem::GPUBackend* backend);
