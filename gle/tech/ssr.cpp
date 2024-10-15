#include "tech/ssr.h"
#include "shape.h"
#include "framebuffer.h"
#include "texture.h"
#include "camera.h"
#include "debug.h"

void tech::ssr::dispatch_ssr_pass(shader& ssr, camera& cam,framebuffer& ssr_buffer, framebuffer& gbuffer, framebuffer& lighting_buffer, glm::vec2 screen_dim)
{
    GPU_MARKER("SSR Pass");
    shapes::s_screen_quad.use();
    ssr_buffer.bind();
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    ssr.use();
    ssr.set_float("SCR_WIDTH", screen_dim.x);
    ssr.set_float("SCR_HEIGHT", screen_dim.y);
    ssr.set_mat4("projection", cam.m_proj);
    ssr.set_mat4("invProjection", glm::inverse(cam.m_proj));
    ssr.set_mat4("rotation", cam.get_rotation_matrix());

    ssr.set_int("gNormal", 0);
    texture::bind_sampler_handle(gbuffer.m_colour_attachments[2], GL_TEXTURE0);

    ssr.set_int("colorBuffer", 1);
    texture::bind_sampler_handle(lighting_buffer.m_colour_attachments.front(), GL_TEXTURE1);

    ssr.set_int("depthMap", 2);
    texture::bind_sampler_handle(gbuffer.m_depth_attachment, GL_TEXTURE2);

    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilMask(0x00);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
    ssr_buffer.unbind();
}
