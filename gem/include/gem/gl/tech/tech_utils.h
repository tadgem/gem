#pragma once
#include "gem/alias.h"
#include "glm.hpp"
#include <string>

namespace gem {
class GLShader;
class GLFramebuffer;

namespace open_gl {

namespace tech {
class Utils {
public:
  static void dispatch_denoise_image(GLShader &denoise_shader,
                                     GLFramebuffer &input,
                                     GLFramebuffer &denoised, float aSigma,
                                     float aThreshold, float aKSigma,
                                     glm::ivec2 window_res);
  static void dispatch_present_image(GLShader &present_shader,
                                     const std::string &uniform_name,
                                     const int texture_slot, gl_handle texture);
  static void blit_to_fb(GLFramebuffer &fb, GLShader &present_shader,
                         const std::string &uniform_name,
                         const int texture_slot, gl_handle texture);
};
} // namespace tech
} // namespace open_gl
} // namespace gem