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
#include "shape.h"
#include <sstream>

struct point_light
{
    glm::vec3   position;
    glm::vec3   colour;
    float       radius;
};


void handle_pbr_forward(shader& pbr_shader, glm::mat4 model_mat, glm::mat3 normal, camera& cam, std::vector<point_light>& lights, model& sponza)
{
    pbr_shader.use();
    glm::mat4 mvp = cam.m_proj * cam.m_view * model_mat;
    pbr_shader.setMat4("model", model_mat);
    pbr_shader.setMat4("projection", cam.m_proj);
    pbr_shader.setMat4("view", cam.m_view);
    pbr_shader.setMat3("normalMatrix", normal);
    pbr_shader.setVec3("camPos", cam.m_pos);

    for (int l = 0; l < lights.size(); l++)
    {
        std::stringstream pos_name;
        pos_name << "lightPositions[" << l << "]";
        std::stringstream col_name;
        col_name << "lightColors[" << l << "]";

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
}

void handle_gbuffer(framebuffer& gbuffer, shader& gbuffer_shader, glm::mat4 mvp, glm::mat4 model_mat, glm::mat3 normal, camera& cam, std::vector<point_light>& lights, model& sponza)
{
    gbuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gbuffer_shader.use();
    gbuffer_shader.setMat4("u_mvp", mvp);
    gbuffer_shader.setMat4("u_model", model_mat);
    gbuffer_shader.setMat4("u_normal", normal);
    gbuffer_shader.setInt("u_diffuse_map", 0);
    gbuffer_shader.setInt("u_normal_map", 1);
    gbuffer_shader.setInt("u_metallic_map", 2);
    gbuffer_shader.setInt("u_roughness_map", 3);
    gbuffer_shader.setInt("u_ao_map", 4);
    for (auto& entry : sponza.m_meshes)
    {
        auto& maps = sponza.m_materials[entry.m_material_index].m_material_maps;
        entry.m_vao.use();
        auto diffuse_tex = maps[texture_map_type::diffuse];
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
    gbuffer.unbind();
}

void handle_present_image(shader& present_shader, const std::string& uniform_name, const int texture_slot, gl_handle texture)
{
    present_shader.use();
    shapes::s_screen_quad.m_vao.use();
    present_shader.setInt(uniform_name.c_str(), texture_slot);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void handle_light_pass(shader& lighting_shader, framebuffer& gbuffer, camera& cam, std::vector<point_light>& point_lights)
{
    lighting_shader.use();
    shapes::s_screen_quad.m_vao.use();

    lighting_shader.setInt("u_diffuse_map", 0);
    lighting_shader.setInt("u_position_map", 1);
    lighting_shader.setInt("u_normal_map", 2);
    lighting_shader.setInt("u_pbr_map", 3);
    
    lighting_shader.setVec3("u_cam_pos", cam.m_pos);
    int num_point_lights = std::min((int)point_lights.size(), 16);

    for (int i = 0; i < num_point_lights; i++)
    {
        std::stringstream pos_name;
        pos_name << "u_point_lights[" << i << "].position";
        std::stringstream col_name;
        col_name << "u_point_lights[" << i << "].colour";
        std::stringstream rad_name;
        rad_name << "u_point_lights[" << i << "].radius";

        lighting_shader.setVec3(pos_name.str(), point_lights[i].position);
        lighting_shader.setVec3(col_name.str(), point_lights[i].colour);
        lighting_shader.setFloat(rad_name.str(), point_lights[i].radius);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer.m_colour_attachments[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer.m_colour_attachments[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer.m_colour_attachments[2]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer.m_colour_attachments[3]);

    // bind all maps
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main()
{
    engine::init();

    std::string pbr_vert = utils::load_string_from_path("assets/shaders/pbr.vert.glsl");
    std::string pbr_frag = utils::load_string_from_path("assets/shaders/pbr.frag.glsl");

    std::string gbuffer_vert = utils::load_string_from_path("assets/shaders/gbuffer.vert.glsl");
    std::string gbuffer_frag = utils::load_string_from_path("assets/shaders/gbuffer.frag.glsl");
    std::string gbuffer_lighting_frag = utils::load_string_from_path("assets/shaders/lighting.frag.glsl");

    std::string present_vert = utils::load_string_from_path("assets/shaders/present.vert.glsl");
    std::string present_frag = utils::load_string_from_path("assets/shaders/present.frag.glsl");

    shader pbr_shader(pbr_vert, pbr_frag);
    shader gbuffer_shader(gbuffer_vert, gbuffer_frag);
    shader lighting_shader(present_vert, gbuffer_lighting_frag);
    shader present_shader(present_vert, present_frag);

    camera cam{};
    model sponza = model::load_model_from_path("assets/models/sponza/Sponza.gltf");
    framebuffer gbuffer{};
    gbuffer.bind();
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT0, 1280, 720, GL_RGBA, GL_NEAREST, GL_UNSIGNED_BYTE);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT1, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT2, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT3, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);

    gbuffer.add_depth_attachment(1280, 720);
    gbuffer.check();
    gbuffer.unbind();

    std::vector<point_light> lights;
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

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

        cam.update(engine::get_window_dim());
        glCullFace(GL_BACK);
        
        glm::mat4 mvp = cam.m_proj * cam.m_view * model;
        //handle_pbr_forward(pbr_shader, model, normal, cam, lights, sponza);

        handle_gbuffer(gbuffer, gbuffer_shader, mvp, model, normal, cam, lights, sponza);

        // handle_present_image(present_shader, "u_image_sampler", 0, gbuffer.m_colour_attachments[00]);
        handle_light_pass(lighting_shader, gbuffer, cam, lights);

        {
            ImGui::Begin("Hello, world!");                          
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::Separator();
            ImGui::DragFloat3("Camera Position", &cam.m_pos[0]);
            ImGui::DragFloat3("Camera Euler", &cam.m_euler[0]);
            ImGui::Separator();
            for (int l = 0; l < lights.size(); l++)
            {
                std::stringstream name;
                name << "Light " << l;
                ImGui::PushID(l);
                if (ImGui::TreeNode(name.str().c_str()))
                {
                    ImGui::DragFloat3("Position", &lights[l].position[0]);
                    ImGui::DragFloat3("Colour", &lights[l].colour[0]);
                    ImGui::DragFloat("Radius", &lights[l].radius);
                    ImGui::TreePop();
                }
                ImGui::PopID();

            }



            ImGui::End();
        }


        engine::engine_post_frame();
    }
    engine::engine_shut_down();

    return 0;
}