#define GLM_ENABLE_EXPERIMENTAL

#include "gem/gl/tech/taa.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/tech/tech_utils.h"
#include "gem/profile.h"
#include "gem/shape.h"
#include "gem/texture.h"

namespace gem {
namespace open_gl {
void tech::taa::dispatch_taa_pass(gl_shader &taa, gl_framebuffer &pass_buffer,
                                  gl_framebuffer pass_resolve_buffer,
                                  gl_framebuffer &pass_history_buffer,
                                  gl_handle &velocity_buffer_attachment,
                                  glm::ivec2 window_res) {
  ZoneScoped;
  GEM_GPU_MARKER("TAA Pass");
  pass_resolve_buffer.bind();
  shapes::s_screen_quad.use();
  taa.use();
  taa.set_vec2("u_resolution", {window_res.x, window_res.y});
  taa.set_int("u_current_buffer", 0);
  texture::bind_sampler_handle(pass_buffer.m_colour_attachments.front(),
                               GL_TEXTURE0);
  taa.set_int("u_history_buffer", 1);
  texture::bind_sampler_handle(pass_history_buffer.m_colour_attachments.front(),
                               GL_TEXTURE1);
  taa.set_int("u_velocity_buffer", 2);
  texture::bind_sampler_handle(velocity_buffer_attachment, GL_TEXTURE2);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  pass_resolve_buffer.unbind();
  texture::bind_sampler_handle(0, GL_TEXTURE0);
  texture::bind_sampler_handle(0, GL_TEXTURE1);
  texture::bind_sampler_handle(0, GL_TEXTURE2);
}
} // namespace open_gl
} // namespace gem