#include <iostream>
#include "imgui.h"

#include "gl.h"
#include "texture.h" 
#include "shader.h"
#include "vertex.h"
#include "utils.h"
#include "model.h"
#include "camera.h"
#include "framebuffer.h"

int main()
{
    engine::init_gl_sdl();
    
    std::string texture_vert = utils::load_string_from_path("assets/shaders/texture.vert.glsl");
    std::string texture_frag = utils::load_string_from_path("assets/shaders/texture.frag.glsl");

    std::string gbuffer_vert = utils::load_string_from_path("assets/shaders/gbuffer.vert.glsl");
    std::string gbuffer_frag = utils::load_string_from_path("assets/shaders/gbuffer.frag.glsl");

    std::string present_vert = utils::load_string_from_path("assets/shaders/present.vert.glsl");
    std::string present_frag = utils::load_string_from_path("assets/shaders/present.frag.glsl");

    shader texture_shader(texture_vert, texture_frag);
    shader gbuffer_shader(gbuffer_vert, gbuffer_frag);
    shader present_shader(present_vert, present_frag);

    camera cam{};
    model sponza = model::load_model_from_path("assets/models/sponza/Sponza.gltf");
    framebuffer gbuffer{};
    gbuffer.bind();
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT0, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT1, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT2, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_depth_attachment(1280, 720);
    gbuffer.check();
    gbuffer.unbind();

    float vertices[] = {
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f   // top 
    };

    vao_builder builder;

    builder.begin();
    builder.add_vertex_buffer(&vertices[0], 9);
    builder.add_vertex_attribute(0, 3 * sizeof(float), 3);

    VAO new_vao = builder.build();

    glm::mat4 model = utils::get_model_matrix(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.2f));

    glEnable(GL_DEPTH_TEST);
    while (!engine::s_quit)
    {
        engine::process_sdl_event();
        engine::engine_pre_frame();        

        texture_shader.use();
        cam.update(engine::get_window_dim());
        glm::mat4 mvp = cam.m_proj * cam.m_view * model;
        texture_shader.setMat4("u_mvp", mvp);
        texture_shader.setInt("uDiffuseSampler", 0);

        for (auto& entry : sponza.m_meshes)
        {
            entry.m_vao.use();
            auto diffuse_tex = sponza.m_materials[entry.m_material_index].m_material_maps[texture_map_type::diffuse];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_tex.m_handle);
            glDrawElements(GL_TRIANGLES, entry.m_index_count, GL_UNSIGNED_INT, 0);
        }

        gbuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gbuffer_shader.use();
        gbuffer_shader.setMat4("u_mvp", mvp);
        gbuffer_shader.setMat4("u_model", model);
        gbuffer_shader.setInt("u_diffuse_sampler", 0);
        for (auto& entry : sponza.m_meshes)
        {
            entry.m_vao.use();
            auto diffuse_tex = sponza.m_materials[entry.m_material_index].m_material_maps[texture_map_type::diffuse];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_tex.m_handle);
            glDrawElements(GL_TRIANGLES, entry.m_index_count, GL_UNSIGNED_INT, 0);
        }
        gbuffer.unbind();

        {
            ImGui::Begin("Hello, world!");                          
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::Separator();
            ImGui::DragFloat3("Camera Position", &cam.m_pos[0]);
            ImGui::DragFloat3("Camera Euler", &cam.m_euler[0]);

            ImGui::End();
        }


        engine::engine_post_frame();
    }
    engine::engine_shut_down();

    return 0;
}