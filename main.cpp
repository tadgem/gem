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
#include "im3d.h"
#include "im3d_gl.h"


struct point_light
{
    glm::vec3   position;
    glm::vec3   colour;
    float       radius;
};

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
    shapes::s_screen_quad.use();
    present_shader.setInt(uniform_name.c_str(), texture_slot);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void handle_light_pass(shader& lighting_shader, framebuffer& gbuffer, camera& cam, std::vector<point_light>& point_lights)
{
    lighting_shader.use();
    shapes::s_screen_quad.use();

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

void OnIm3D(aabb& level_bb, glm::mat4 model_matrix)
{
    aabb bb = utils::transform_aabb(level_bb, model_matrix);
    //aabb bb = level_bb;
    Im3d::PushColor(Im3d::Color_Red);
    Im3d::DrawAlignedBox(
        { bb.min.x, bb.min.y, bb.min.z }, 
        { bb.max.x, bb.max.y, bb.max.z }
    );
}

float get_aabb_area(aabb& bb)
{
    glm::vec3 dim = bb.max - bb.min;
    return glm::length(dim);
}

int main()
{
    engine::init();

    std::string gbuffer_vert = utils::load_string_from_path("assets/shaders/gbuffer.vert.glsl");
    std::string gbuffer_frag = utils::load_string_from_path("assets/shaders/gbuffer.frag.glsl");
    std::string gbuffer_lighting_frag = utils::load_string_from_path("assets/shaders/lighting.frag.glsl");

    std::string debug3dtex_vert = utils::load_string_from_path("assets/shaders/debug_3d_tex.vert.glsl");
    std::string debug3dtex_frag = utils::load_string_from_path("assets/shaders/debug_3d_tex.frag.glsl");

    std::string present_vert = utils::load_string_from_path("assets/shaders/present.vert.glsl");
    std::string present_frag = utils::load_string_from_path("assets/shaders/present.frag.glsl");

    std::string voxelization_compute = utils::load_string_from_path("assets/shaders/gbuffer_voxelization.comp.glsl");

    shader gbuffer_shader(gbuffer_vert, gbuffer_frag);
    shader lighting_shader(present_vert, gbuffer_lighting_frag);
    shader present_shader(present_vert, present_frag);
    shader debug3dtex_shader(debug3dtex_vert, debug3dtex_frag);
    shader voxelization(voxelization_compute);


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

    framebuffer lightpass_buffer{};
    lightpass_buffer.bind();
    lightpass_buffer.add_colour_attachment(GL_COLOR_ATTACHMENT0, 1280, 720, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    lightpass_buffer.check();
    lightpass_buffer.unbind();

    std::vector<point_light> lights;
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

    constexpr int _3d_tex_res = 128;
    constexpr int _3d_cube_res = _3d_tex_res * _3d_tex_res * _3d_tex_res;

    float* _3d_tex_data = new float[_3d_cube_res * 4];
    std::vector<glm::mat4> instance_matrices;
    std::vector<glm::vec3> instance_uvs;


    for (auto i = 0; i < _3d_cube_res * 4; i++)
    {
        //float r = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
        float r = 0.0f;
        _3d_tex_data[i] = r;
    }

    for (auto i = 0; i < _3d_cube_res; i++)
    {
        //create a new VAO for debug cubes
        // first vbo is same as cube
        // instance vbo is per-instance transform
        float x = i / (_3d_tex_res * _3d_tex_res);
        float y = (i / _3d_tex_res) % _3d_tex_res;
        float z = i % _3d_tex_res;

        instance_uvs.push_back({ (z + 1) / _3d_tex_res,(y + 1) / _3d_tex_res, (x + 1)/ _3d_tex_res });
        instance_matrices.push_back(utils::get_model_matrix({ z,y,x }, {0,90,0}, glm::vec3(1.0f)));
    }
    VAO instanced_cubes = shapes::gen_cube_instanced_vao(instance_matrices, instance_uvs);
    texture _3d_tex = texture::create_3d_texture({ _3d_tex_res, _3d_tex_res, _3d_tex_res }, GL_RGBA, GL_RGBA32F, GL_FLOAT, _3d_tex_data);
    glAssert(glBindImageTexture(0, _3d_tex.m_handle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));

    voxelization.use();
    voxelization.setInt("u_gbuffer_pos", 0);
    voxelization.setInt("u_gbuffer_lighting", 1);
    voxelization.setVec3("u_voxel_resolution", { _3d_tex_res, _3d_tex_res, _3d_tex_res });
    voxelization.setVec3("u_aabb.min", sponza.m_aabb.min);
    voxelization.setVec3("u_aabb.max", sponza.m_aabb.max);
    
    glm::mat4 model = utils::get_model_matrix(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.2f));
    glm::mat3 normal = utils::get_normal_matrix(model);

    bool draw_debug_3d_texture = true;

    auto im3d_s =  im3d_gl::load_im3d();

    while (!engine::s_quit)
    {
        glEnable(GL_DEPTH_TEST);
        engine::process_sdl_event();
        engine::engine_pre_frame();        
        glm::mat4 mvp = cam.m_proj * cam.m_view * model;
        cam.update(engine::get_window_dim());
        im3d_gl::new_frame_im3d(im3d_s);
        
        // compute
        voxelization.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbuffer.m_colour_attachments[1]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lightpass_buffer.m_colour_attachments[0]);
        glAssert(glDispatchCompute(128, 72, 1));

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        
        handle_gbuffer(gbuffer, gbuffer_shader, mvp, model, normal, cam, lights, sponza);
        lightpass_buffer.bind();
        handle_light_pass(lighting_shader, gbuffer, cam, lights);
        lightpass_buffer.unbind();
                
        shapes::s_screen_quad.use();
        present_shader.use();
        present_shader.setInt("u_image_sampler", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightpass_buffer.m_colour_attachments.front());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        if(draw_debug_3d_texture)
        {
            instanced_cubes.use();
            debug3dtex_shader.use();
            debug3dtex_shader.setMat4("viewProjection", cam.m_proj * cam.m_view);
            debug3dtex_shader.setInt("u_volume", 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, _3d_tex.m_handle);
            glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT,  0, _3d_cube_res);
        }
        {
            OnIm3D(sponza.m_aabb, model);
        }
        {
            ImGui::Begin("Hello, world!");                          
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::Separator();
            ImGui::DragFloat3("Camera Position", &cam.m_pos[0]);
            ImGui::DragFloat3("Camera Euler", &cam.m_euler[0]);
            ImGui::Separator();
            ImGui::DragFloat3("Level Bounding Box Min", &sponza.m_aabb.min[0]);
            ImGui::DragFloat3("Level Bounding Box Max", &sponza.m_aabb.max[0]);
            ImGui::Text("Level Bounding Volume Area %.2f", get_aabb_area(sponza.m_aabb));
            glm::vec3 dim = sponza.m_aabb.max - sponza.m_aabb.min;
            ImGui::Text("Level Bounding Volume Dimensions %.2f,%.2f,%.2f", dim.x, dim.y, dim.z);
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

        im3d_gl::end_frame_im3d(im3d_s, {1280, 720}, cam);
        engine::engine_post_frame();
    }
    im3d_gl::shutdown_im3d(im3d_s);
    engine::engine_shut_down();

    return 0;
}