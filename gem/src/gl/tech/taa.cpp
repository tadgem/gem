#define GLM_ENABLE_EXPERIMENTAL

#include "gem/gl/tech/taa.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/tech/tech_utils.h"
#include "gem/profile.h"
#include "gem/shape.h"
#include "gem/texture.h"

namespace gem {
namespace gl {
void tech::TemporalAntiAliasing::DispatchTAAPass(GLShader &taa, GLFramebuffer &pass_buffer,
                                  GLFramebuffer pass_resolve_buffer,
                                  GLFramebuffer &pass_history_buffer,
                                  gl_handle &velocity_buffer_attachment,
                                  glm::ivec2 window_res) {
  ZoneScoped;
  GEM_GPU_MARKER("TAA Pass");
  pass_resolve_buffer.Bind();
  Shapes::s_screen_quad.Use();
  taa.Use();
  taa.SetVec2f("u_resolution", {window_res.x, window_res.y});
  taa.SetInt("u_current_buffer", 0);
  Texture::BindSamplerHandle(pass_buffer.m_colour_attachments.front(),
                               GL_TEXTURE0);
  taa.SetInt("u_history_buffer", 1);
  Texture::BindSamplerHandle(pass_history_buffer.m_colour_attachments.front(),
                               GL_TEXTURE1);
  taa.SetInt("u_velocity_buffer", 2);
  Texture::BindSamplerHandle(velocity_buffer_attachment, GL_TEXTURE2);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  pass_resolve_buffer.Unbind();
  Texture::BindSamplerHandle(0, GL_TEXTURE0);
  Texture::BindSamplerHandle(0, GL_TEXTURE1);
  Texture::BindSamplerHandle(0, GL_TEXTURE2);
}
} // namespace open_gl
} // namespace gem