#pragma once
#include <SDL.h>
#undef main
#include "gem/dbg_memory.h"
#include "glm.hpp"
#include "imgui.h"
#include <iostream>
#include <string>
namespace gem {

enum class backend_api { open_gl, vulkan };

void set_imgui_style();

void init_built_in_assets();

void init_imgui_file_dialog();

struct backend_init {
  glm::vec2 window_resolution;
  bool enable_vsync;

  GEM_IMPL_ALLOC(backend_init)
};

class gpu_backend {
protected:
  float p_frametime = 0.0f;
  uint64_t m_now_counter, m_last_counter;

private:
  inline static backend_api s_selected_backend_api;
  inline static gpu_backend *s_selected_backend = nullptr;

public:
  virtual void init(backend_init &init_props) = 0;
  virtual void process_sdl_event() = 0;
  virtual void engine_pre_frame() = 0;
  virtual void engine_post_frame() = 0;
  virtual void engine_shut_down() = 0;
  virtual void engine_handle_input_events(SDL_Event &input_event) = 0;
  virtual glm::vec2 get_window_dim() = 0;
  virtual const backend_api get_backend_api_enum() = 0;

  float get_frame_time() const { return p_frametime; }

  SDL_Window *m_window = nullptr;
  bool m_quit = false;
  ImGuiIO *m_imgui_io = nullptr;

  static gpu_backend *selected() { return s_selected_backend; }
  static backend_api get_backend_api() { return s_selected_backend_api; }

  template <typename _Backend, typename... Args>
  static void init_backend(backend_init &props, Args &&...args) {
    static_assert(std::is_base_of<gpu_backend, _Backend>());
    auto *backend = new _Backend(std::forward<Args>(args)...);
    backend->init(props);
    s_selected_backend = static_cast<gpu_backend *>(backend);
    s_selected_backend_api = s_selected_backend->get_backend_api_enum();
  }
};
} // namespace gem