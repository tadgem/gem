#define GLM_ENABLE_EXPERIMENTAL

#include "gem/gl/tech/ssr.h"
#include "gem/camera.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/profile.h"
#include "gem/shape.h"
#include "gem/texture.h"
namespace gem {
namespace open_gl {

void tech::ssr::dispatch_ssr_pass(gl_shader &ssr, camera &cam,
                                  gl_framebuffer &ssr_buffer,
                                  gl_framebuffer &gbuffer,
                                  gl_framebuffer &lighting_buffer,
                                  glm::vec2 screen_dim) {
  ZoneScoped;
  GPU_MARKER("SSR Pass");
  shapes::s_screen_quad.use();
  ssr_buffer.bind();
  glClearColor(0, 0, 0, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDepthMask(GL_FALSE);

  ssr.use();
  ssr.set_float("u_screen_width", screen_dim.x);
  ssr.set_float("u_screen_height", screen_dim.y);
  ssr.set_mat4("u_inv_projection", glm::inverse(cam.m_proj));
  ssr.set_mat4("u_projection", cam.m_proj);
  ssr.set_mat4("u_rotation", cam.get_rotation_matrix());

  ssr.set_int("u_gnormal_buffer", 0);
  texture::bind_sampler_handle(gbuffer.m_colour_attachments[2], GL_TEXTURE0);

  ssr.set_int("u_gcolour_buffer", 1);
  texture::bind_sampler_handle(lighting_buffer.m_colour_attachments.front(),
                               GL_TEXTURE1);

  ssr.set_int("u_gpbr_buffer", 2);
  texture::bind_sampler_handle(gbuffer.m_colour_attachments[3], GL_TEXTURE2);

  ssr.set_int("u_depth_buffer", 3);
  texture::bind_sampler_handle(gbuffer.m_depth_attachment, GL_TEXTURE3);

  glStencilFunc(GL_EQUAL, 1, 0xFF);
  glStencilMask(0x00);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glDepthMask(GL_TRUE);
  ssr_buffer.unbind();
}
} // namespace open_gl
} // namespace gem