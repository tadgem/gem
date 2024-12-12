#pragma once
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/gl_shader.h"

namespace gem {

class Camera;
class Scene;
class AssetManager;
namespace open_gl {
namespace tech {
class GBuffer {
public:
  static void dispatch_gbuffer_with_id(u32 frame_index, GLFramebuffer &gbuffer,
                                       GLFramebuffer &previous_position_buffer,
                                       GLShader &gbuffer_shader,
                                       AssetManager &am, Camera &cam,
                                       std::vector<Scene *> &scenes,
                                       glm::ivec2 win_res);

  static void dispatch_gbuffer_textureless_with_id(
      u32 frame_index, GLFramebuffer &gbuffer,
      GLFramebuffer &previous_position_buffer,
      GLShader &gbuffer_textureless_shader, AssetManager &am, Camera &cam,
      std::vector<Scene *> &scenes, glm::ivec2 win_res);
};
} // namespace tech
} // namespace open_gl
} // namespace gem