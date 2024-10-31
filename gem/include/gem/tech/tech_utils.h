#pragma once
#include "gem/alias.h"
#include "glm.hpp"
#include <string>

namespace gem {

class shader;
class framebuffer;

namespace tech {
class utils {
public:
  static void dispatch_denoise_image(shader &denoise_shader, framebuffer &input,
                                     framebuffer &denoised, float aSigma,
                                     float aThreshold, float aKSigma,
                                     glm::ivec2 window_res);
  static void dispatch_present_image(shader &present_shader,
                                     const std::string &uniform_name,
                                     const int texture_slot, gl_handle texture);
  static void blit_to_fb(framebuffer &fb, shader &present_shader,
                         const std::string &uniform_name,
                         const int texture_slot, gl_handle texture);
};
} // namespace tech
} // namespace gem