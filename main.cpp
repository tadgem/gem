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
#include <sstream>

struct light
{
    glm::vec3 position;
    glm::vec3 colour;
};

int main()
{
    engine::init_gl_sdl();

    std::string pbr_vert = utils::load_string_from_path("assets/shaders/pbr.vert.glsl");
    std::string pbr_frag = utils::load_string_from_path("assets/shaders/pbr.frag.glsl");

    std::string gbuffer_vert = utils::load_string_from_path("assets/shaders/gbuffer.vert.glsl");
    std::string gbuffer_frag = utils::load_string_from_path("assets/shaders/gbuffer.frag.glsl");

    std::string present_vert = utils::load_string_from_path("assets/shaders/present.vert.glsl");
    std::string present_frag = utils::load_string_from_path("assets/shaders/present.frag.glsl");

    shader pbr_shader(pbr_vert, pbr_frag);
    shader gbuffer_shader(gbuffer_vert, gbuffer_frag);
    shader present_shader(present_vert, present_frag);

    camera cam{};
    model sponza = model::load_model_from_path("assets/models/sponza/Sponza.gltf");
    framebuffer gbuffer{};
    gbuffer.bind();
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT0, 1280, 720, GL_RGBA, GL_NEAREST, GL_UNSIGNED_BYTE);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT1, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT2, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_depth_attachment(1280, 720);
    gbuffer.check();
    gbuffer.unbind();

    std::vector<light> lights;
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0} });
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0} });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0} });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} });

    constexpr int _3d_tex_res = 128;
    constexpr int _3d_cube_res = _3d_tex_res * _3d_tex_res * _3d_tex_res;

    float* _3d_tex_data = new float[_3d_cube_res * 4];

    for (auto i = 0; i < _3d_cube_res; i++)
    {
        float r = rand() * 10000000;
        _3d_tex_data[i] = r;
    }

    texture _3d_tex = texture::create_3d_texture({ 128, 128, 128 }, GL_RGBA, GL_RGBA32F, GL_FLOAT, _3d_tex_data);
    

    glm::mat4 model = utils::get_model_matrix(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.2f));
    glm::mat3 normal = utils::get_normal_matrix(model);

    glEnable(GL_DEPTH_TEST);
    while (!engine::s_quit)
    {
        engine::process_sdl_event();
        engine::engine_pre_frame();        
        glCullFace(GL_BACK);
        pbr_shader.use();
        cam.update(engine::get_window_dim());
        glm::mat4 mvp = cam.m_proj * cam.m_view * model;
        pbr_shader.setMat4("model", model);
        pbr_shader.setMat4("projection", cam.m_proj);
        pbr_shader.setMat4("view", cam.m_view);
        pbr_shader.setMat3("normalMatrix", normal);
        pbr_shader.setVec3("camPos", cam.m_pos);

        for (int l = 0; l < lights.size(); l++)
        {
            std::stringstream pos_name;
            pos_name << "lightPositions[" << l + 1 << "]";
            std::stringstream col_name;
            col_name << "lightColors[" << l + 1 << "]";

            pbr_shader.setVec3(pos_name.str(), lights[l].position);
            pbr_shader.setVec3(col_name.str(), lights[l].colour);
        }

        pbr_shader.setInt("albedoMap", 0);
        pbr_shader.setInt("normalMap", 1);
        pbr_shader.setInt("metallicMap", 2);
        pbr_shader.setInt("roughnessMap", 3);
        pbr_shader.setInt("aoMap", 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture::white->m_handle);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture::black->m_handle);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture::black->m_handle);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture::white->m_handle);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, texture::white->m_handle);

        for (auto& entry : sponza.m_meshes)
        {
            auto& maps = sponza.m_materials[entry.m_material_index].m_material_maps;
            entry.m_vao.use();
            auto diffuse_tex = sponza.m_materials[entry.m_material_index].m_material_maps[texture_map_type::diffuse];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_tex.m_handle);
            if (maps.find(texture_map_type::normal) != maps.end())
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, maps[texture_map_type::normal].m_handle);
            }
            if (maps.find(texture_map_type::metallicness) != maps.end())
            {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, maps[texture_map_type::metallicness].m_handle);
            }
            if (maps.find(texture_map_type::roughness) != maps.end())
            {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, maps[texture_map_type::roughness].m_handle);
            }
            if (maps.find(texture_map_type::ao) != maps.end())
            {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, maps[texture_map_type::metallicness].m_handle);
            }
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
            auto& maps = sponza.m_materials[entry.m_material_index].m_material_maps;
            entry.m_vao.use();
            auto diffuse_tex = maps[texture_map_type::diffuse];
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
            ImGui::Separator();
            for (int l = 0; l < lights.size(); l++)
            {
                std::stringstream pos_name;
                pos_name << "lightPositions[" << l + 1 << "]";
                std::stringstream col_name;
                col_name << "lightColors[" << l + 1 << "]";

                ImGui::DragFloat3(pos_name.str().c_str(), &lights[l].position[0]);
                ImGui::DragFloat3(col_name.str().c_str(), &lights[l].colour[0]);
            }



            ImGui::End();
        }


        engine::engine_post_frame();
    }
    engine::engine_shut_down();

    return 0;
}