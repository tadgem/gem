#pragma once
#include "gem/gl/gl_shader.h"

namespace gem {
class GLFramebuffer;

namespace gl {
namespace tech {
class TemporalAntiAliasing {
public:
  static void DispatchTAAPass(GLShader &taa, GLFramebuffer &pass_buffer,
                                GLFramebuffer pass_resolve_buffer,
                                GLFramebuffer &pass_history_buffer,
                                gl_handle &velocity_buffer_attachment,
                                glm::ivec2 window_res);
};
}
}
}