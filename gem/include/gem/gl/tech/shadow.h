#pragma once
#include "gem/lights.h"
#include "gem/shader.h"

namespace gem {

class gl_framebuffer;
class scene;

namespace tech {
class shadow {
public:
  static void dispatch_shadow_pass(gl_framebuffer &shadow_fb,
                                   shader &shadow_shader, dir_light &sun,
                                   std::vector<scene *> &scenes,
                                   glm::ivec2 window_res);
};
} // namespace tech
} // namespace gem