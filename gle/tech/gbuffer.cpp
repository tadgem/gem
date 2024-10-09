#include "tech/gbuffer.h"
#include "camera.h"
#include "scene.h"
#include "transform.h"
#include "mesh.h"
#include "material.h"

void tech::gbuffer::dispatch_gbuffer(u32 frame_index, framebuffer& gbuffer, framebuffer& previous_position_buffer, shader& gbuffer_shader, camera& cam,  scene& current_scene, glm::ivec2 win_res)
{
    gbuffer.bind();
    glm::mat4 current_vp = cam.m_proj * cam.m_view;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    gbuffer_shader.use();
    gbuffer_shader.set_vec2("u_resolution", { win_res.x, win_res.y });
    gbuffer_shader.set_mat4("u_vp", current_vp);
    gbuffer_shader.set_mat4("u_last_vp", cam.m_last_vp);
    gbuffer_shader.set_int("u_frame_index", frame_index);
    gbuffer_shader.set_int("u_diffuse_map", 0);
    gbuffer_shader.set_int("u_normal_map", 1);
    gbuffer_shader.set_int("u_metallic_map", 2);
    gbuffer_shader.set_int("u_roughness_map", 3);
    gbuffer_shader.set_int("u_ao_map", 4);
    gbuffer_shader.set_int("u_prev_position_map", 5);

    texture::bind_sampler_handle(0, GL_TEXTURE0);
    texture::bind_sampler_handle(0, GL_TEXTURE1);
    texture::bind_sampler_handle(0, GL_TEXTURE2);
    texture::bind_sampler_handle(0, GL_TEXTURE3);
    texture::bind_sampler_handle(0, GL_TEXTURE4);

    texture::bind_sampler_handle(previous_position_buffer.m_colour_attachments.front(), GL_TEXTURE5);

    auto renderables = current_scene.m_registry.view<transform, mesh, material>();

    for (auto [e, trans, emesh, ematerial] : renderables.each())
    {
        ematerial.bind_material_uniforms();
        gbuffer_shader.set_mat4("u_model", trans.m_model);
        gbuffer_shader.set_mat4("u_last_model", trans.m_last_model);
        gbuffer_shader.set_mat4("u_normal", trans.m_normal_matrix);
        emesh.m_vao.use();
        glDrawElements(GL_TRIANGLES, emesh.m_index_count, GL_UNSIGNED_INT, 0);
    }
    gbuffer.unbind();
}