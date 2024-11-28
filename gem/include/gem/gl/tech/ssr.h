#pragma once
#include "gem/shader.h"
namespace gem {

class camera;
class gl_framebuffer;
namespace tech {
class ssr {
public:
  static void dispatch_ssr_pass(shader &ssr, camera &cam,
                                gl_framebuffer &ssr_buffer,
                                gl_framebuffer &gbuffer,
                                gl_framebuffer &lighting_buffer,
                                glm::vec2 screen_dim);
};
} // namespace tech
} // namespace gem