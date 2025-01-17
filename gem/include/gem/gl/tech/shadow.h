#pragma once
#include <vector>
#include "gem/gl/gl_shader.h"
#include "gem/lights.h"
namespace gem {

class GLFramebuffer;
class Scene;
namespace open_gl {
namespace tech {
class Shadow {
public:
  static void dispatch_shadow_pass(GLFramebuffer &shadow_fb,
                                   GLShader &shadow_shader,
                                   DirectionalLight &sun,
                                   std::vector<Scene *> &scenes,
                                   glm::ivec2 window_res);
};
} // namespace tech
} // namespace open_gl
} // namespace gem