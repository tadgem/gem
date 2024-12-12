#pragma once
#include "gem/gl/gl_shader.h"

namespace gem {
class GLFramebuffer;

namespace open_gl {
namespace tech {
class TemporalAntiAliasing {
public:
  static void dispatch_taa_pass(GLShader &taa, GLFramebuffer &pass_buffer,
                                GLFramebuffer pass_resolve_buffer,
                                GLFramebuffer &pass_history_buffer,
                                gl_handle &velocity_buffer_attachment,
                                glm::ivec2 window_res);
};
} // namespace tech
} // namespace open_gl
} // namespace gem