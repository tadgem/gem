#include "tech/taa.h"
#include "framebuffer.h"
#include "texture.h"
#include "shape.h"
#include "tech/tech_utils.h"

void tech::taa::dispatch_taa_pass(shader& taa, framebuffer& pass_buffer, framebuffer pass_resolve_buffer, framebuffer& pass_history_buffer, gl_handle& velocity_buffer_attachment, glm::ivec2 window_res)
{
    pass_resolve_buffer.bind();
    shapes::s_screen_quad.use();
    taa.use();
    taa.set_vec2("u_resolution", { window_res.x, window_res.y });
    taa.set_int("u_current_light_buffer", 0);
    texture::bind_sampler_handle(pass_buffer.m_colour_attachments.front(), GL_TEXTURE0);
    taa.set_int("u_history_light_buffer", 1);
    texture::bind_sampler_handle(pass_history_buffer.m_colour_attachments.front(), GL_TEXTURE1);
    taa.set_int("u_velocity_buffer", 2);
    texture::bind_sampler_handle(velocity_buffer_attachment, GL_TEXTURE2);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    pass_resolve_buffer.unbind();
    texture::bind_sampler_handle(0, GL_TEXTURE0);
    texture::bind_sampler_handle(0, GL_TEXTURE1);
    texture::bind_sampler_handle(0, GL_TEXTURE2);
}