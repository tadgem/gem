#pragma once
#include "gem/gl/gl_shader.h"
#include "gem/lights.h"
#include <vector>

namespace gem {
class GLFramebuffer;
class Camera;

namespace gl {
namespace tech {
class PBRLighting {
public:
  static void
  DispatchLightPass(GLShader &lighting_shader,
                                  GLFramebuffer &lighting_buffer,
                                  GLFramebuffer &gbuffer,
                                  GLFramebuffer &dir_light_shadow_buffer, Camera &cam,
                      std::vector<PointLight> &point_lights,
                                  DirectionalLight &sun);
};
} // namespace tech
} // namespace open_gl
} // namespace gem