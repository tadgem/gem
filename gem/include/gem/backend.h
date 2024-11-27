#pragma once
#include <SDL.h>
#undef main
#include "GL/glew.h"
#include "glm.hpp"
#include "imgui.h"
#include <iostream>
#include <string>
GLenum glCheckError_(const char *file, int line);
#define glAssert(X)                                                            \
  X;                                                                           \
  glCheckError_(__FILE__, __LINE__)

namespace gem {

struct backend_init {
  glm::vec2 window_resolution;
  bool enable_vsync;
};

class gpu_backend
{
protected:
  float         p_frametime = 0.0f;
private:
  inline static gpu_backend*  s_selected_backend = nullptr;
public:
  virtual void      init(backend_init& init_props) = 0;
  virtual void      process_sdl_event() = 0;
  virtual void      engine_pre_frame() = 0;
  virtual void      engine_post_frame() = 0;
  virtual void      engine_shut_down() = 0;
  virtual void      engine_handle_input_events(SDL_Event &input_event) = 0;
  virtual glm::vec2 get_window_dim() = 0;

  float             get_frame_time() const { return p_frametime; }

  SDL_Window *  m_window = nullptr;
  bool          m_quit = false;
  ImGuiIO *     m_imgui_io = nullptr;

  static gpu_backend*  selected() { return s_selected_backend;}

  template<typename _Backend, typename... Args>
  static void  init_backend(backend_init& props, Args&&... args)
  {
    static_assert(std::is_base_of<gpu_backend, _Backend>());
    auto* backend = new _Backend(std::forward<Args>(args)...);
    backend->init(props);
    s_selected_backend = static_cast<gpu_backend*>(backend);
  }

};

class gl_backend : public gpu_backend
{
public:
  void init(backend_init &init_props) override;
  void process_sdl_event() override;
  void engine_pre_frame() override;
  void engine_post_frame() override;
  void engine_shut_down() override;
  void engine_handle_input_events(SDL_Event &input_event) override;
  glm::vec2 get_window_dim() override;

};
} // namespace gem