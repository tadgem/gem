#pragma once
#include "gem/gl/gl_shader.h"
namespace gem {

class Camera;
class GLFramebuffer;
namespace open_gl {
namespace tech {
class ScreenSpaceReflections {
public:
  static void dispatch_ssr_pass(GLShader &ssr, Camera &cam,
                                GLFramebuffer &ssr_buffer,
                                GLFramebuffer &gbuffer,
                                GLFramebuffer &lighting_buffer,
                                glm::vec2 screen_dim);
};
} // namespace tech
} // namespace open_gl
} // namespace gem