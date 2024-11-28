#pragma once
#include "gem/gl/gl_shader.h"
#include "gem/lights.h"

namespace gem {

class gl_framebuffer;
class scene;
namespace open_gl {
namespace tech {
class shadow {
public:
  static void dispatch_shadow_pass(gl_framebuffer &shadow_fb,
                                   gl_shader &shadow_shader, dir_light &sun,
                                   std::vector<scene *> &scenes,
                                   glm::ivec2 window_res);
};
} // namespace tech
} // namespace open_gl
} // namespace gem