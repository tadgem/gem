#pragma once
#include "gem/gl/gl_shader.h"
namespace gem {

class camera;
class gl_framebuffer;
namespace open_gl {
namespace tech {
class ssr {
public:
  static void dispatch_ssr_pass(gl_shader &ssr, camera &cam,
                                gl_framebuffer &ssr_buffer,
                                gl_framebuffer &gbuffer,
                                gl_framebuffer &lighting_buffer,
                                glm::vec2 screen_dim);
};
} // namespace tech
} // namespace open_gl
} // namespace gem