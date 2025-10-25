#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/tech/tech_utils.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/gl_shader.h"
#include "gem/profile.h"
#include "gem/shape.h"
#include "gem/texture.h"

namespace gem {
namespace gl {
void tech::Utils::DispatchDenoiseImage(GLShader &denoise_shader,
                                         GLFramebuffer &input,
                                         GLFramebuffer &denoised, float aSigma,
                                         float aThreshold, float aKSigma,
                                         glm::ivec2 window_res) {
  ZoneScoped;
  GEM_GPU_MARKER("Denoise Image Pass");
  denoised.Bind();
  denoise_shader.Use();
  denoise_shader.SetInt("imageData", 0);
  denoise_shader.SetFloat("uSigma", aSigma);
  denoise_shader.SetFloat("uThreshold", aThreshold);
  denoise_shader.SetFloat("uKSigma", aKSigma);
  denoise_shader.SetVec2f("wSize", {window_res.x, window_res.y});
  Texture::BindSamplerHandle(input.colour_attachments.front(), GL_TEXTURE0);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  denoised.Unbind();
}

void tech::Utils::DispatchPresentImage(GLShader &present_shader,
                                         const std::string &uniform_name,
                                         const int texture_slot,
                                         gl_handle texture) {
  ZoneScoped;
  GEM_GPU_MARKER("Present Image Pass");
  present_shader.Use();
  Shapes::kScreenQuad.Use();
  present_shader.SetInt(uniform_name.c_str(), texture_slot);
  Texture::BindSamplerHandle(texture, GL_TEXTURE0 + texture_slot);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  Texture::BindSamplerHandle(0, GL_TEXTURE0);
}

void tech::Utils::DispatchBlitToFB(GLFramebuffer &fb, GLShader &present_shader,
                             const std::string &uniform_name,
                             const int texture_slot, gl_handle texture) {
  ZoneScoped;
  GEM_GPU_MARKER("Blit Pass");
  fb.Bind();
  tech::Utils::DispatchPresentImage(present_shader, uniform_name,
                                      texture_slot, texture);
  fb.Unbind();
}
} // namespace open_gl
} // namespace gem