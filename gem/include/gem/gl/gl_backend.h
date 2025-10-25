#pragma once
#include "GL/glew.h"
#include "gem/backend.h"

namespace gem {
class GLBackend : public GPUBackend {
public:
  void Init(BackendInit &init_props) override;
  void ProcessEvents() override;
  void PreFrame() override;
  void PostFrame() override;
  void ShutDown() override;
  void HandleInputEvents(SDL_Event &input_event) override;
  virtual const BackendAPI GetAPI() {
    return BackendAPI::open_gl;
  };
  glm::vec2 GetWindowDimensions() override;

  SDL_GLContext *gl_context;
};
} // namespace gem