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
#include "voxelisation.h"

#include <sstream>
#include "im3d.h"
#include "im3d_gl.h"

static glm::vec3 custom_orientation;

struct dir_light
{
    glm::vec3   direction;
    glm::vec3   colour;
    float       intensity = 1.0f;
};

struct point_light
{
    glm::vec3   position;
    glm::vec3   colour;
    float       radius;
    float       intensity = 1.0f;
};

Im3d::Vec3 ToIm3D(glm::vec3& input)
{
    return { input.x, input.y, input.z };
}

static glm::mat4 last_vp = glm::mat4(1.0);

inline static int frame_index = 0;
void handle_gbuffer(framebuffer& gbuffer, framebuffer& previous_position_buffer, shader& gbuffer_shader, glm::mat4 mvp, glm::mat4 model_mat, glm::mat3 normal, camera& cam, std::vector<point_light>& lights, model& sponza)
{
    frame_index++;
    gbuffer.bind();
    glm::mat4 current_vp = cam.m_proj * cam.m_view;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gbuffer_shader.use();
    gbuffer_shader.setVec2("u_resolution", {1920.0, 1080.0});
    gbuffer_shader.setMat4("u_vp", current_vp);
    gbuffer_shader.setMat4("u_model", model_mat);
    gbuffer_shader.setMat4("u_last_vp", last_vp);
    gbuffer_shader.setMat4("u_last_model", model_mat);
    gbuffer_shader.setMat4("u_normal", normal);
    gbuffer_shader.setInt("u_frame_index", frame_index);
    gbuffer_shader.setInt("u_diffuse_map", 0);
    gbuffer_shader.setInt("u_normal_map", 1);
    gbuffer_shader.setInt("u_metallic_map", 2);
    gbuffer_shader.setInt("u_roughness_map", 3);
    gbuffer_shader.setInt("u_ao_map", 4);
    gbuffer_shader.setInt("u_prev_position_map", 5);

    texture::bind_handle(previous_position_buffer.m_colour_attachments.front(), GL_TEXTURE5);
    for (auto& entry : sponza.m_meshes)
    {
        auto& maps = sponza.m_materials[entry.m_material_index].m_material_maps;
        entry.m_vao.use();
        maps[texture_map_type::diffuse].bind(GL_TEXTURE0);

        if (maps.find(texture_map_type::normal) != maps.end())
        {
            texture::bind_handle(maps[texture_map_type::normal].m_handle, GL_TEXTURE1);
        }
        if (maps.find(texture_map_type::metallicness) != maps.end())
        {
            texture::bind_handle(maps[texture_map_type::metallicness].m_handle, GL_TEXTURE2);
        }
        if (maps.find(texture_map_type::roughness) != maps.end())
        {
            texture::bind_handle(maps[texture_map_type::roughness].m_handle, GL_TEXTURE3);
        }
        if (maps.find(texture_map_type::ao) != maps.end())
        {
            texture::bind_handle(maps[texture_map_type::ao].m_handle, GL_TEXTURE4);
        }

        glDrawElements(GL_TRIANGLES, entry.m_index_count, GL_UNSIGNED_INT, 0);
    }
    gbuffer.unbind();
    last_vp = current_vp;
}

void handle_present_image(shader& present_shader, const std::string& uniform_name, const int texture_slot, gl_handle texture)
{
    present_shader.use();
    shapes::s_screen_quad.use();
    present_shader.setInt(uniform_name.c_str(), texture_slot);
    texture::bind_handle(texture, GL_TEXTURE0 + texture_slot);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void blit_to_fb(framebuffer& fb, shader& present_shader, const std::string& uniform_name, const int texture_slot, gl_handle texture)
{
    fb.bind();
    handle_present_image(present_shader, uniform_name, texture_slot, texture);
    fb.unbind();
}

void handle_light_pass(shader& lighting_shader, framebuffer& gbuffer, camera& cam, std::vector<point_light>& point_lights, dir_light& sun)
{
    lighting_shader.use();
    shapes::s_screen_quad.use();

    lighting_shader.setInt("u_diffuse_map", 0);
    lighting_shader.setInt("u_position_map", 1);
    lighting_shader.setInt("u_normal_map", 2);
    lighting_shader.setInt("u_pbr_map", 3);
    
    lighting_shader.setVec3("u_cam_pos", cam.m_pos);

    lighting_shader.setVec3("u_dir_light.direction", utils::get_forward(sun.direction));
    lighting_shader.setVec3("u_dir_light.colour", sun.colour);
    lighting_shader.setFloat("u_dir_light.intensity", sun.intensity);

    int num_point_lights = std::min((int)point_lights.size(), 16);

    for (int i = 0; i < num_point_lights; i++)
    {
        std::stringstream pos_name;
        pos_name << "u_point_lights[" << i << "].position";
        std::stringstream col_name;
        col_name << "u_point_lights[" << i << "].colour";
        std::stringstream rad_name;
        rad_name << "u_point_lights[" << i << "].radius";
        std::stringstream int_name;
        int_name << "u_point_lights[" << i << "].intensity";

        lighting_shader.setVec3(pos_name.str(), point_lights[i].position);
        lighting_shader.setVec3(col_name.str(), point_lights[i].colour);
        lighting_shader.setFloat(rad_name.str(), point_lights[i].radius);
        lighting_shader.setFloat(int_name.str(), point_lights[i].intensity);
    }

    texture::bind_handle(gbuffer.m_colour_attachments[0], GL_TEXTURE0);
    texture::bind_handle(gbuffer.m_colour_attachments[1], GL_TEXTURE1);
    texture::bind_handle(gbuffer.m_colour_attachments[2], GL_TEXTURE2);
    texture::bind_handle(gbuffer.m_colour_attachments[3], GL_TEXTURE3);

    // bind all maps
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void draw_arrow(glm::vec3 pos, glm::vec3 dir)
{
    glm::vec3 end = pos + dir;
    Im3d::DrawArrow(ToIm3D(pos), ToIm3D(end));
    Im3d::DrawSphere(ToIm3D(end), 0.25f);
}

void OnIm3D(aabb& level_bb)
{
    //aabb bb = level_bb;
    Im3d::DrawAlignedBox(
        { level_bb.min.x, level_bb.min.y, level_bb.min.z },
        { level_bb.max.x, level_bb.max.y, level_bb.max.z }
    );
}

float get_aabb_area(aabb& bb)
{
    return glm::length(bb.max - bb.min) ;
}

int main()
{
    engine::init();
    custom_orientation = glm::vec3(0, 1, 0);
    glm::ivec2 window_res{ 1920, 1080 };
    std::string gbuffer_vert = utils::load_string_from_path("assets/shaders/gbuffer.vert.glsl");
    std::string gbuffer_frag = utils::load_string_from_path("assets/shaders/gbuffer.frag.glsl");
    std::string gbuffer_floats_frag = utils::load_string_from_path("assets/shaders/gbuffer_floats.frag.glsl");
    std::string gbuffer_lighting_frag = utils::load_string_from_path("assets/shaders/lighting.frag.glsl");

    std::string visualize_3dtex_vert = utils::load_string_from_path("assets/shaders/visualize_3d_tex.vert.glsl");
    std::string visualize_3dtex_frag = utils::load_string_from_path("assets/shaders/visualize_3d_tex.frag.glsl");

    std::string present_vert = utils::load_string_from_path("assets/shaders/present.vert.glsl");
    std::string present_frag = utils::load_string_from_path("assets/shaders/present.frag.glsl");

    std::string voxelization_compute = utils::load_string_from_path("assets/shaders/gbuffer_voxelization.comp.glsl");
    std::string voxel_cone_tracing_frag = utils::load_string_from_path("assets/shaders/voxel_cone_tracing.frag.glsl");

    std::string taa_frag = utils::load_string_from_path("assets/shaders/taa.frag.glsl");
    std::string denoise_frag = utils::load_string_from_path("assets/shaders/denoise.frag.glsl");
    std::string gi_combine_frag = utils::load_string_from_path("assets/shaders/gi_combine.frag.glsl");


    shader gbuffer_shader(gbuffer_vert, gbuffer_frag);
    shader gbuffer_floats_shader(gbuffer_vert, gbuffer_floats_frag);
    shader lighting_shader(present_vert, gbuffer_lighting_frag);
    shader present_shader(present_vert, present_frag);
    shader visualize_3dtex(visualize_3dtex_vert, visualize_3dtex_frag);
    shader voxelization(voxelization_compute);
    shader voxel_cone_tracing(present_vert, voxel_cone_tracing_frag);
    shader taa(present_vert, taa_frag);
    shader denoise(present_vert, denoise_frag);
    shader gi_combine(present_vert, gi_combine_frag);

    camera cam{};
    model sponza = model::load_model_from_path("assets/models/sponza/Sponza.gltf");
    framebuffer gbuffer{};
    gbuffer.bind();
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA, GL_NEAREST, GL_UNSIGNED_BYTE);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT1, window_res.x, window_res.y, GL_RGBA32F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT2, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT3, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT4, window_res.x, window_res.y, GL_RG16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT5, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    gbuffer.add_colour_attachment(GL_COLOR_ATTACHMENT6, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);


    gbuffer.add_depth_attachment(window_res.x, window_res.y);
    gbuffer.check();
    gbuffer.unbind();

    framebuffer lightpass_buffer{};
    lightpass_buffer.bind();
    lightpass_buffer.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    lightpass_buffer.check();
    lightpass_buffer.unbind();

    framebuffer lightpass_buffer_resolve{};
    lightpass_buffer_resolve.bind();
    lightpass_buffer_resolve.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    lightpass_buffer_resolve.check();
    lightpass_buffer_resolve.unbind();

    framebuffer history_buffer_lighting{};
    history_buffer_lighting.bind();
    history_buffer_lighting.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA16F, GL_LINEAR, GL_FLOAT);
    history_buffer_lighting.check();
    history_buffer_lighting.unbind();

    framebuffer history_buffer_position{};
    history_buffer_position.bind();
    history_buffer_position.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA32F, GL_NEAREST, GL_FLOAT);
    history_buffer_position.check();
    history_buffer_position.unbind();

    framebuffer history_buffer_conetracing{};
    history_buffer_conetracing.bind();
    history_buffer_conetracing.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    history_buffer_conetracing.check();
    history_buffer_conetracing.unbind();

    framebuffer buffer_conetracing{};
    buffer_conetracing.bind();
    buffer_conetracing.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x , window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    buffer_conetracing.check();
    buffer_conetracing.unbind();

    framebuffer buffer_conetracing_denoise{};
    buffer_conetracing_denoise.bind();
    buffer_conetracing_denoise.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    buffer_conetracing_denoise.check();
    buffer_conetracing_denoise.unbind();

    framebuffer buffer_conetracing_resolve{};
    buffer_conetracing_resolve.bind();
    buffer_conetracing_resolve.add_colour_attachment(GL_COLOR_ATTACHMENT0, window_res.x, window_res.y, GL_RGBA16F, GL_NEAREST, GL_FLOAT);
    buffer_conetracing_resolve.check();
    buffer_conetracing_resolve.unbind();

    dir_light dir
    {
        {156.0f, -3.0f, 0.0f},
        {1.0f,1.0f,1.0f},
        2.75f
    };
    std::vector<point_light> lights;
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

    constexpr int _3d_tex_res = 256;

    glm::mat4 model = utils::get_model_matrix(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.2f));
    glm::mat3 normal = utils::get_normal_matrix(model);
    sponza.m_aabb = utils::transform_aabb(sponza.m_aabb, model);

    voxel::grid              voxel_data = voxel::create_grid({ _3d_tex_res, _3d_tex_res, _3d_tex_res }, sponza.m_aabb);
    voxel::grid_visualiser   voxel_visualiser = voxel::create_grid_visualiser(voxel_data, visualize_3dtex, 8);

    glm::vec3 aabb_dim = sponza.m_aabb.max - sponza.m_aabb.min;
    glm::vec3 unit = glm::vec3((aabb_dim.x / _3d_tex_res), (aabb_dim.y / _3d_tex_res), (aabb_dim.z / _3d_tex_res));
    glm::vec3 n_unit = glm::normalize(unit);


    voxelization.use();
    voxelization.setInt("u_gbuffer_pos", 0);
    voxelization.setInt("u_gbuffer_lighting", 1);
    voxelization.setVec3("u_voxel_resolution", glm::vec3( _3d_tex_res));

    bool draw_debug_3d_texture = false;
    bool draw_direct_lighting = true;
    bool draw_direct_lighting_no_taa = true;
    bool draw_cone_tracing_pass = true;
    bool draw_cone_tracing_pass_no_taa = true;
    bool draw_final_pass = true;

    bool draw_im3d = true;

    auto im3d_s =  im3d_gl::load_im3d();

    glm::vec3 aabb_half_extent = (sponza.m_aabb.max - sponza.m_aabb.min) / 2.0f;
    glm::vec3 aabb_center = sponza.m_aabb.min + aabb_half_extent;
    glm::mat4 aabb_model = utils::get_model_matrix(aabb_center, glm::vec3(0.0f), aabb_half_extent * 2.0f);

    GLfloat aSigma = 7.0f;
    GLfloat aThreshold = 0.180f;
    GLfloat aKSigma =  3.0f;
    
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
        voxelization.setVec2("u_input_resolution", { 1920.0, 1080.0 });
        voxelization.setVec3("u_aabb.min", sponza.m_aabb.min);
        voxelization.setVec3("u_aabb.max", sponza.m_aabb.max);
        texture::bind_handle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
        texture::bind_handle(lightpass_buffer.m_colour_attachments[0], GL_TEXTURE1);
        glAssert(glDispatchCompute(window_res.x / 10, window_res.y / 10, 1));
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        
        handle_gbuffer(gbuffer, history_buffer_position, gbuffer_shader, mvp, model, normal, cam, lights, sponza);
        lightpass_buffer.bind();
        handle_light_pass(lighting_shader, gbuffer, cam, lights, dir);
        lightpass_buffer.unbind();
                
        if (draw_direct_lighting)
        {
            lightpass_buffer_resolve.bind();
            shapes::s_screen_quad.use();
            taa.use();
            taa.setVec2("u_resolution", { 1920.0, 1080.0 });
            taa.setInt("u_current_light_buffer", 0);
            texture::bind_handle(lightpass_buffer.m_colour_attachments.front(), GL_TEXTURE0);
            taa.setInt("u_history_light_buffer", 1);
            texture::bind_handle(history_buffer_lighting.m_colour_attachments.front(), GL_TEXTURE1);
            taa.setInt("u_velocity_buffer", 2);
            texture::bind_handle(gbuffer.m_colour_attachments[4], GL_TEXTURE2);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            lightpass_buffer_resolve.unbind();
        }

        if (draw_cone_tracing_pass || draw_cone_tracing_pass_no_taa)
        {
            //glViewport(0, 0, 1920.0 / 2.0, 1080.0 / 2.0);
            shapes::s_screen_quad.use();
            buffer_conetracing.bind();
            voxel_cone_tracing.use();
            voxel_cone_tracing.setVec3("u_aabb.min", sponza.m_aabb.min);
            voxel_cone_tracing.setVec3("u_aabb.max", sponza.m_aabb.max);
            voxel_cone_tracing.setVec3("u_voxel_resolution", glm::vec3(_3d_tex_res));
            voxel_cone_tracing.setInt("u_position_map", 0);

            texture::bind_handle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
            voxel_cone_tracing.setInt("u_normal_map", 1);
            texture::bind_handle(gbuffer.m_colour_attachments[2], GL_TEXTURE1);
            voxel_cone_tracing.setInt("u_voxel_map", 2);
            texture::bind_handle(voxel_data.texture.m_handle, GL_TEXTURE2, GL_TEXTURE_3D);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            buffer_conetracing.unbind();
            //glViewport(0, 0, 1920.0, 1080.0);
        }

        if (draw_direct_lighting)
        {
            handle_present_image(present_shader, "u_image_sampler", 0, lightpass_buffer_resolve.m_colour_attachments.front());
        }

        if (draw_cone_tracing_pass)
        {
            buffer_conetracing_resolve.bind();
            shapes::s_screen_quad.use();
            taa.use();
            taa.setInt("u_current_light_buffer", 0);
            texture::bind_handle(buffer_conetracing.m_colour_attachments.front(), GL_TEXTURE0);
            taa.setInt("u_history_light_buffer", 1);
            texture::bind_handle(history_buffer_conetracing.m_colour_attachments.front(), GL_TEXTURE1);
            taa.setInt("u_velocity_buffer", 2);
            texture::bind_handle(gbuffer.m_colour_attachments[4], GL_TEXTURE2);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            buffer_conetracing_resolve.unbind();

            buffer_conetracing_denoise.bind();
            denoise.use();
            denoise.setInt("imageData", 0);
            denoise.setFloat("uSigma", aSigma);
            denoise.setFloat("uThreshold", aThreshold);
            denoise.setFloat("uKSigma", aKSigma);
            denoise.setVec2("wSize", { 1920.0, 1080.0 });
            texture::bind_handle(buffer_conetracing_resolve.m_colour_attachments.front(), GL_TEXTURE0);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            buffer_conetracing_denoise.unbind();

            handle_present_image(present_shader, "u_image_sampler", 0, buffer_conetracing_denoise.m_colour_attachments.front());
        }
        if (draw_cone_tracing_pass_no_taa)
        {
            handle_present_image(present_shader, "u_image_sampler", 0, buffer_conetracing.m_colour_attachments.front());
        }
        if (draw_direct_lighting_no_taa)
        {
            handle_present_image(present_shader, "u_image_sampler", 0, lightpass_buffer.m_colour_attachments.front());

        }

        // copy history buffers
        blit_to_fb(history_buffer_lighting, present_shader, "u_image_sampler", 0, lightpass_buffer_resolve.m_colour_attachments[0]);
        blit_to_fb(history_buffer_position, present_shader, "u_image_sampler", 0, gbuffer.m_colour_attachments[1]);
        blit_to_fb(history_buffer_conetracing, present_shader, "u_image_sampler", 0, buffer_conetracing_denoise.m_colour_attachments.front());
        glClear(GL_DEPTH_BUFFER_BIT);

        if (draw_debug_3d_texture)
        {
            voxel_visualiser.texel_shape.use();
            auto& vs = voxel_visualiser.visual_shader;
            vs.use();
            vs.setMat4("u_view_projection", cam.m_proj* cam.m_view);
            vs.setIVec3("u_texture_resolution", voxel_data.resolution);
            vs.setVec3("u_aabb.min", sponza.m_aabb.min);
            vs.setVec3("u_aabb.max", sponza.m_aabb.max);
            vs.setIVec3("u_voxel_group_resolution", glm::ivec3(voxel_visualiser.texel_resolution));
            vs.setInt("u_volume", 0);
            texture::bind_handle(voxel_data.texture.m_handle, GL_TEXTURE0, GL_TEXTURE_3D);
            glDrawElementsInstanced(GL_TRIANGLES, voxel_visualiser.index_count, GL_UNSIGNED_INT, 0, voxel_visualiser.total_invocations);
        }

        if (draw_final_pass)
        {
            shapes::s_screen_quad.use();
            gi_combine.use();
            gi_combine.setInt("lighting_pass", 0);
            texture::bind_handle(lightpass_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE0);
            gi_combine.setInt("cone_tracing_pass", 1);
            texture::bind_handle(buffer_conetracing_denoise.m_colour_attachments.front(), GL_TEXTURE1);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        {
            ImGui::Begin("Hello, world!");                          
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::Separator();
            ImGui::Checkbox("Render 3D Voxel Grid", &draw_debug_3d_texture);
            ImGui::Checkbox("Render Final Pass", &draw_final_pass);
            ImGui::Checkbox("Render Direct Lighting Pass", &draw_direct_lighting);
            ImGui::Checkbox("Render Direct Lighting Pass NO TAA", &draw_direct_lighting_no_taa);
            ImGui::Checkbox("Render Cone Tracing Pass", &draw_cone_tracing_pass);
            ImGui::Checkbox("Render Cone Tracing Pass NO TAA", &draw_cone_tracing_pass_no_taa);
            ImGui::Checkbox("Render IM3D", &draw_im3d);
            ImGui::Separator();
            ImGui::Text("Denoise properties");
            ImGui::DragFloat("Sigma", &aSigma);
            ImGui::DragFloat("Threshold", &aThreshold);
            ImGui::DragFloat("KSigma", &aKSigma);
            ImGui::Separator();

            ImGui::DragFloat3("Camera Position", &cam.m_pos[0]);
            ImGui::DragFloat3("Camera Euler", &cam.m_euler[0]);
            ImGui::Separator();
            ImGui::DragFloat3("Orientation Test", &custom_orientation[0]);
            ImGui::Separator();
            // ImGui::DragFloat3("Level Bounding Box Min", &sponza.m_aabb.min[0]);
            // ImGui::DragFloat3("Level Bounding Box Max", &sponza.m_aabb.max[0]);
            ImGui::Text("Level Bounding Volume Area %.2f", get_aabb_area(sponza.m_aabb));
            glm::vec3 dim = sponza.m_aabb.max - sponza.m_aabb.min;
            ImGui::Text("Level Bounding Volume Dimensions %.2f,%.2f,%.2f", dim.x, dim.y, dim.z);
            ImGui::Separator();
            ImGui::Text("Lights");
            ImGui::ColorEdit3("Dir Light Colour", &dir.colour[0]);
            ImGui::DragFloat3("Dir Light Rotation", &dir.direction[0]);
            ImGui::DragFloat("Dir Light Intensity", &dir.intensity);

            for (int l = 0; l < lights.size(); l++)
            {
                std::stringstream name;
                name << "Light " << l;
                ImGui::PushID(l);
                if (ImGui::TreeNode(name.str().c_str()))
                {
                    ImGui::DragFloat3("Position", &lights[l].position[0]);
                    ImGui::ColorEdit3("Colour", &lights[l].colour[0]);
                    ImGui::DragFloat("Radius", &lights[l].radius);
                    ImGui::DragFloat("Intensity", &lights[l].intensity);
                    ImGui::TreePop();
                }
                ImGui::PopID();

            }

            ImGui::End();
        }
        if (draw_im3d)
        {
            im3d_gl::end_frame_im3d(im3d_s, {window_res.x, window_res.y}, cam);
        }
        else
        {
            Im3d::EndFrame();
        }
        engine::engine_post_frame();
    }
    im3d_gl::shutdown_im3d(im3d_s);
    engine::engine_shut_down();

    return 0;
}