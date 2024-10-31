#define GLM_ENABLE_EXPERIMENTAL
#include "gem/tech/tech_utils.h"
#include "gem/debug.h"
#include "gem/framebuffer.h"
#include "gem/profile.h"
#include "gem/shader.h"
#include "gem/shape.h"
#include "gem/texture.h"

namespace gem {

void tech::utils::dispatch_denoise_image(shader &denoise_shader,
                                         framebuffer &input,
                                         framebuffer &denoised, float aSigma,
                                         float aThreshold, float aKSigma,
                                         glm::ivec2 window_res) {
  ZoneScoped;
  GPU_MARKER("Denoise Image Pass");
  denoised.bind();
  denoise_shader.use();
  denoise_shader.set_int("imageData", 0);
  denoise_shader.set_float("uSigma", aSigma);
  denoise_shader.set_float("uThreshold", aThreshold);
  denoise_shader.set_float("uKSigma", aKSigma);
  denoise_shader.set_vec2("wSize", {window_res.x, window_res.y});
  texture::bind_sampler_handle(input.m_colour_attachments.front(), GL_TEXTURE0);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  denoised.unbind();
}

void tech::utils::dispatch_present_image(shader &present_shader,
                                         const std::string &uniform_name,
                                         const int texture_slot,
                                         gl_handle texture) {
  ZoneScoped;
  GPU_MARKER("Present Image Pass");
  present_shader.use();
  shapes::s_screen_quad.use();
  present_shader.set_int(uniform_name.c_str(), texture_slot);
  texture::bind_sampler_handle(texture, GL_TEXTURE0 + texture_slot);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  texture::bind_sampler_handle(0, GL_TEXTURE0);
}

void tech::utils::blit_to_fb(framebuffer &fb, shader &present_shader,
                             const std::string &uniform_name,
                             const int texture_slot, gl_handle texture) {
  ZoneScoped;
  GPU_MARKER("Blit Pass");
  fb.bind();
  tech::utils::dispatch_present_image(present_shader, uniform_name,
                                      texture_slot, texture);
  fb.unbind();
}
} // namespace gem