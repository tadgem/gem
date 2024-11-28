#pragma once
#include "gem/gl/gl_shader.h"

namespace gem {
class gl_framebuffer;

namespace open_gl {
namespace tech {
class taa {
public:
  static void dispatch_taa_pass(gl_shader &taa, gl_framebuffer &pass_buffer,
                                gl_framebuffer pass_resolve_buffer,
                                gl_framebuffer &pass_history_buffer,
                                gl_handle &velocity_buffer_attachment,
                                glm::ivec2 window_res);
};
} // namespace tech
} // namespace open_gl
} // namespace gem