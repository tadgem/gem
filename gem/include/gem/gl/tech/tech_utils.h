#pragma once
#include "gem/alias.h"
#include "glm.hpp"
#include <string>

namespace gem {
class gl_shader;
class gl_framebuffer;

namespace open_gl {

namespace tech {
class utils {
public:
  static void dispatch_denoise_image(gl_shader &denoise_shader,
                                     gl_framebuffer &input,
                                     gl_framebuffer &denoised, float aSigma,
                                     float aThreshold, float aKSigma,
                                     glm::ivec2 window_res);
  static void dispatch_present_image(gl_shader &present_shader,
                                     const std::string &uniform_name,
                                     const int texture_slot, gl_handle texture);
  static void blit_to_fb(gl_framebuffer &fb, gl_shader &present_shader,
                         const std::string &uniform_name,
                         const int texture_slot, gl_handle texture);
};
} // namespace tech
} // namespace open_gl
} // namespace gem