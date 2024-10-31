#pragma once
#include "gem/lights.h"
#include "gem/shader.h"
#include <vector>

namespace gem {
class framebuffer;
class camera;

namespace tech {
class lighting {
public:
  static void
  dispatch_light_pass(shader &lighting_shader, framebuffer &lighting_buffer,
                      framebuffer &gbuffer,
                      framebuffer &dir_light_shadow_buffer, camera &cam,
                      std::vector<point_light> &point_lights, dir_light &sun);
};
} // namespace tech
} // namespace gem