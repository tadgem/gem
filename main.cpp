#include <iostream>
#include "imgui.h"

#include "gl.h"
#include "texture.h" 
#include "shader.h"
#include "vertex.h"
#include "utils.h"
#include "model.h"
#include "camera.h"
#include <ext/matrix_transform.hpp>

int main()
{
    engine::init_gl_sdl();

    
    std::string texture_vert = utils::load_string_from_path("assets/shaders/texture.vert.glsl");
    std::string texture_frag = utils::load_string_from_path("assets/shaders/texture.frag.glsl");

    shader texture(texture_vert, texture_frag);
    camera cam{};
    model sponza = model::load_model_from_path("assets/models/sponza/Sponza.gltf");


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
        

        texture.use();
        cam.update(engine::get_window_dim());
        glm::mat4 mvp = cam.m_proj * cam.m_view * model;
        texture.setMat4("u_mvp", mvp);
        texture.setInt("uDiffuseSampler", 0);

        for (auto& entry : sponza.m_meshes)
        {
            entry.m_vao.use();
            auto diffuse_tex = sponza.m_materials[entry.m_material_index].m_material_maps[texture_map_type::diffuse];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_tex.m_handle);
            glDrawElements(GL_TRIANGLES, entry.m_index_count, GL_UNSIGNED_INT, 0);
        }

        // update shader uniform

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