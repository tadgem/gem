#pragma once
#include "GL/glew.h"
#include "gem/backend.h"

namespace gem {
class gl_backend : public gpu_backend {
public:
  void init(backend_init &init_props) override;
  void process_sdl_event() override;
  void engine_pre_frame() override;
  void engine_post_frame() override;
  void engine_shut_down() override;
  void engine_handle_input_events(SDL_Event &input_event) override;
  virtual const backend_api get_backend_api_enum() {
    return backend_api::open_gl;
  };
  glm::vec2 get_window_dim() override;

  SDL_GLContext *m_sdl_gl_context;
};
} // namespace gem