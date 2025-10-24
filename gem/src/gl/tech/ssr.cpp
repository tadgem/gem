#define GLM_ENABLE_EXPERIMENTAL

#include "gem/gl/tech/ssr.h"
#include "gem/camera.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/profile.h"
#include "gem/shape.h"
#include "gem/texture.h"
namespace gem {
namespace gl {

void tech::ScreenSpaceReflections::DispatchSSRPass(GLShader &ssr, Camera &cam,
                                  GLFramebuffer &ssr_buffer,
                                  GLFramebuffer &gbuffer,
                                  GLFramebuffer &lighting_buffer,
                                  glm::vec2 screen_dim) {
  ZoneScoped;
  GEM_GPU_MARKER("SSR Pass");
  Shapes::s_screen_quad.Use();
  ssr_buffer.Bind();
  glClearColor(0, 0, 0, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDepthMask(GL_FALSE);

  ssr.Use();
  ssr.SetFloat("u_screen_width", screen_dim.x);
  ssr.SetFloat("u_screen_height", screen_dim.y);
  ssr.SetMat4f("u_inv_projection", glm::inverse(cam.m_proj));
  ssr.SetMat4f("u_projection", cam.m_proj);
  ssr.SetMat4f("u_rotation", cam.GetRotationMatrix());

  ssr.SetInt("u_gnormal_buffer", 0);
  Texture::BindSamplerHandle(gbuffer.m_colour_attachments[2], GL_TEXTURE0);

  ssr.SetInt("u_gcolour_buffer", 1);
  Texture::BindSamplerHandle(lighting_buffer.m_colour_attachments.front(),
                               GL_TEXTURE1);

  ssr.SetInt("u_gpbr_buffer", 2);
  Texture::BindSamplerHandle(gbuffer.m_colour_attachments[3], GL_TEXTURE2);

  ssr.SetInt("u_depth_buffer", 3);
  Texture::BindSamplerHandle(gbuffer.m_depth_attachment, GL_TEXTURE3);

  glStencilFunc(GL_EQUAL, 1, 0xFF);
  glStencilMask(0x00);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glDepthMask(GL_TRUE);
  ssr_buffer.Unbind();
}
} // namespace open_gl
} // namespace gem