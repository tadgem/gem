#pragma once
#include "gem/gl/gl_shader.h"
#include "gem/lights.h"
#include <vector>

namespace gem {
class gl_framebuffer;
class camera;

namespace tech {
class lighting {
public:
  static void
  dispatch_light_pass(gl_shader &lighting_shader, gl_framebuffer &lighting_buffer,
                      gl_framebuffer &gbuffer,
                      gl_framebuffer &dir_light_shadow_buffer, camera &cam,
                      std::vector<point_light> &point_lights, dir_light &sun);
};
} // namespace tech
} // namespace gem