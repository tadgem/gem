#pragma once
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/gl_shader.h"

namespace gem {

class camera;
class scene;
class asset_manager;
namespace open_gl {
namespace tech {
class gbuffer {
public:
  static void dispatch_gbuffer(u32 frame_index, gl_framebuffer &gbuffer,
                               gl_framebuffer &previous_position_buffer,
                               gl_shader &gbuffer_shader, asset_manager &am,
                               camera &cam, std::vector<scene *> &scenes,
                               glm::ivec2 win_res);

  static void dispatch_gbuffer_with_id(u32 frame_index, gl_framebuffer &gbuffer,
                                       gl_framebuffer &previous_position_buffer,
                                       gl_shader &gbuffer_shader,
                                       asset_manager &am, camera &cam,
                                       std::vector<scene *> &scenes,
                                       glm::ivec2 win_res);
};
} // namespace tech
} // namespace open_gl
} // namespace gem