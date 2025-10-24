#pragma once
#include "gem/alias.h"
#include "glm.hpp"
#include <string>

namespace gem {
class GLShader;
class GLFramebuffer;

namespace gl {

namespace tech {
class Utils {
public:
  static void DispatchDenoiseImage(GLShader &denoise_shader,
                                     GLFramebuffer &input,
                                     GLFramebuffer &denoised, float aSigma,
                                     float aThreshold, float aKSigma,
                                     glm::ivec2 window_res);
  static void DispatchPresentImage(GLShader &present_shader,
                                     const std::string &uniform_name,
                                     const int texture_slot, gl_handle texture);
  static void DispatchBlitToFB(GLFramebuffer &fb, GLShader &present_shader,
                         const std::string &uniform_name,
                         const int texture_slot, gl_handle texture);
};
} // namespace tech
} // namespace open_gl
} // namespace gem