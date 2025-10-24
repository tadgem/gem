#pragma once
#include "gem/gl/gl_shader.h"
namespace gem {

class Camera;
class GLFramebuffer;
namespace gl {
namespace tech {
class ScreenSpaceReflections {
public:
  static void DispatchSSRPass(GLShader &ssr, Camera &cam,
                                GLFramebuffer &ssr_buffer,
                                GLFramebuffer &gbuffer,
                                GLFramebuffer &lighting_buffer,
                                glm::vec2 screen_dim);
};
} // namespace tech
} // namespace open_gl
} // namespace gem