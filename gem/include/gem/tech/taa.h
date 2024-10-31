#pragma once
#include "gem/shader.h"

namespace gem {
class framebuffer;

namespace tech {
class taa {
public:
  static void dispatch_taa_pass(shader &taa, framebuffer &pass_buffer,
                                framebuffer pass_resolve_buffer,
                                framebuffer &pass_history_buffer,
                                gl_handle &velocity_buffer_attachment,
                                glm::ivec2 window_res);
};
} // namespace tech
} // namespace gem