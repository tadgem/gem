#include <iostream>
#include "imgui.h"

#include "gl.h"
#include "texture.h" 
#include "shader.h"
#include "material.h"
#include "vertex.h"
#include "utils.h"
#include "model.h"
#include "camera.h"
#include "framebuffer.h"
#include "shape.h"
#include "voxelisation.h"
#include "asset_manager.h"
#include "renderer.h"

#include <sstream>
#include "im3d.h"
#include "im3d_gl.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "input.h"
#include "json.hpp"
#include "scene.h"
#include "asset.h"
#include "events.h"
#include "lights.h"
#include "transform.h"
#include "im3d_math.h"
using namespace nlohmann;
static glm::vec3 custom_orientation;


Im3d::Vec3 ToIm3D(glm::vec3& input)
{
    return { input.x, input.y, input.z };
}

static glm::mat4 last_vp = glm::mat4(1.0);
inline static constexpr int shadow_resolution = 4096;
inline static constexpr int _3d_tex_res = 256;
const float SCREEN_W = 1920.0;
const float SCREEN_H = 1080.0;
inline static entt::entity selected_entity = entt::entity(UINT32_MAX);


void on_im3d(scene& current_scene, camera& cam, int& selected_entity)
{
    if (!current_scene.does_entity_exist(selected_entity))
    {
        selected_entity = -1;
        return;
    }

    entt::entity e = static_cast<entt::entity>(selected_entity);
    if (!current_scene.m_registry.any_of<mesh>(e))
    {
        selected_entity = -1;
        return;
    }
    mesh& meshc = current_scene.m_registry.get<mesh>(e);
    transform& trans = current_scene.m_registry.get<transform>(e);

    Im3d::DrawAlignedBox(ToIm3D(meshc.m_transformed_aabb.min), ToIm3D(meshc.m_transformed_aabb.max));
}

float get_aabb_area(aabb& bb)
{
    return glm::length(bb.max - bb.min) ;
}

int main()
{
    glm::ivec2 window_res{ SCREEN_W, SCREEN_H };
    engine::init(window_res);
    asset_manager am{};

    am.load_asset("assets/shaders2/gbuffer.shader", asset_type::shader);
    am.load_asset("assets/shaders2/lighting.shader", asset_type::shader);
    am.load_asset("assets/shaders2/visualize_3d_tex.shader", asset_type::shader);
    am.load_asset("assets/shaders2/present.shader", asset_type::shader);
    am.load_asset("assets/shaders2/dir_light_shadow.shader", asset_type::shader);
    am.load_asset("assets/shaders2/voxel_cone_tracing.shader", asset_type::shader);
    am.load_asset("assets/shaders2/ssr.shader", asset_type::shader);
    am.load_asset("assets/shaders2/taa.shader", asset_type::shader);
    am.load_asset("assets/shaders2/denoise.shader", asset_type::shader);
    am.load_asset("assets/shaders2/gi_combine.shader", asset_type::shader);
    am.load_asset("assets/shaders2/downsample.shader", asset_type::shader);
    am.load_asset("assets/shaders2/gbuffer_voxelization.shader", asset_type::shader);
    am.load_asset("assets/shaders2/voxel_mips.shader", asset_type::shader);
    gl_renderer_builtin renderer{};

    renderer.init(am);

    custom_orientation = glm::vec3(0, 1, 0);


    camera cam{};
    debug_camera_controller controller{};
    scene scene("test_scene");
    entity e = scene.create_entity("Daddalus");
    e.has_component<entity_data>();
    entity_data& data = e.get_component<entity_data>();
    material mat(renderer.m_gbuffer_shader->m_data);

    e.add_component<material>(renderer.m_gbuffer_shader->m_data);

    model sponza_geo = model::load_model_and_textures_from_path("assets/models/sponza/Sponza.gltf");

    scene.create_entity_from_model(sponza_geo, renderer.m_gbuffer_shader->m_data, glm::vec3(0.03), glm::vec3(0.0, 0.0, 0.0),
        {
            {"u_diffuse_map", texture_map_type::diffuse},
            {"u_normal_map", texture_map_type::normal},
            {"u_metallic_map", texture_map_type::metallicness},
            {"u_roughness_map", texture_map_type::roughness},
            {"u_ao_map", texture_map_type::ao}
        });

    


    dir_light dir
    {
        {90.01f, 0.0f, 0.0f},
        {1.0f,1.0f,1.0f},
        2.75f
    };

    entity dir_light_entity = scene.create_entity("dir light");
    e.add_component<dir_light>(dir);
    std::vector<point_light> lights;
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});


    constexpr glm::vec3 _3d_tex_res_vec = { _3d_tex_res, _3d_tex_res, _3d_tex_res };

    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 euler = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(0.1f);
    glm::mat4 model = utils::get_model_matrix(pos, euler, scale);
    glm::mat3 normal = utils::get_normal_matrix(model);
    sponza_geo.m_aabb = utils::transform_aabb(sponza_geo.m_aabb, model);


    glm::vec3 aabb_dim = sponza_geo.m_aabb.max - sponza_geo.m_aabb.min;
    glm::vec3 unit = glm::vec3((aabb_dim.x / _3d_tex_res), (aabb_dim.y / _3d_tex_res), (aabb_dim.z / _3d_tex_res));
    glm::vec3 n_unit = glm::normalize(unit);


    
    glm::vec3 aabb_half_extent = (sponza_geo.m_aabb.max - sponza_geo.m_aabb.min) / 2.0f;
    glm::vec3 aabb_center = sponza_geo.m_aabb.min + aabb_half_extent;
    glm::mat4 aabb_model = utils::get_model_matrix(aabb_center, glm::vec3(0.0f), aabb_half_extent * 2.0f);

    GLfloat aSigma = 2.0f;
    GLfloat aThreshold = 0.1f;
    GLfloat aKSigma =  2.0f;
    GLfloat vxgi_cone_distance = get_aabb_area(sponza_geo.m_aabb) / 10.0f;
    GLfloat diffuse_spec_mix = 0.0;
    float brightness = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.05f;
    while (!engine::s_quit)
    {
        glm::mat4 model = utils::get_model_matrix(pos, euler, scale);

        glEnable(GL_DEPTH_TEST);
        
        engine::process_sdl_event();
        engine::engine_pre_frame();      
        renderer.pre_frame(cam, scene);
        glm::vec2 window_dim = engine::get_window_dim();
        controller.update(window_dim, cam);
        cam.update(window_dim);
        scene.on_update();
        renderer.render(cam, scene);



        glm::vec2 mouse_pos = input::get_mouse_position();

        if (input::get_mouse_button(mouse_button::left) && !ImGui::GetIO().WantCaptureMouse)
        {
            selected_entity = renderer.get_mouse_entity(mouse_pos);
        }

        if (scene.does_entity_exist((u32) selected_entity))
        {
            entity_data& data = scene.m_registry.get<entity_data>((entt::entity)selected_entity);
            ImGui::Begin(data.m_name.c_str());

            // do each component ImGui

            ImGui::End();
        }


        {
            ImGui::Begin("VXGI Debug");
            ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
            ImGui::Text("Selected Entity ID : %d", selected_entity);
            ImGui::Separator();
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::Separator();
            /*ImGui::Checkbox("Render 3D Voxel Grid", &draw_debug_3d_texture);
            ImGui::Checkbox("Render Final Pass", &draw_final_pass);
            ImGui::Checkbox("Render Direct Lighting Pass", &draw_direct_lighting);
            ImGui::Checkbox("Render Direct Lighting Pass NO TAA", &draw_direct_lighting_no_taa);
            ImGui::Checkbox("Render Cone Tracing Pass", &draw_cone_tracing_pass);
            ImGui::Checkbox("Render SSR", &draw_ssr);
            ImGui::Checkbox("Render Cone Tracing Pass NO TAA", &draw_cone_tracing_pass_no_taa);
            ImGui::Checkbox("Render IM3D", &draw_im3d);*/
            ImGui::Separator();
            ImGui::Text("Brightness / Contrast / Saturation");
            ImGui::DragFloat("Brightness", &brightness);
            ImGui::DragFloat("Contrast", &contrast);
            ImGui::DragFloat("Saturation", &saturation);
            ImGui::Separator();
            ImGui::Text("VXGI Settings");
            ImGui::DragFloat("Trace Distance", &vxgi_cone_distance);
            ImGui::DragFloat("Diffuse / Spec Mix", &diffuse_spec_mix, 1.0f, 0.0f, 1.0f);
            ImGui::Separator();
            ImGui::Text("Denoise Settings");
            ImGui::DragFloat("Sigma", &aSigma);
            ImGui::DragFloat("Threshold", &aThreshold);
            ImGui::DragFloat("KSigma", &aKSigma);
            ImGui::Separator();

            ImGui::DragFloat3("Camera Position", &cam.m_pos[0]);
            ImGui::DragFloat3("Camera Euler", &cam.m_euler[0]);
            ImGui::Separator();
            ImGui::DragFloat3("Orientation Test", &custom_orientation[0]);
            ImGui::Separator();
            ImGui::Text("Level Bounding Volume Area %.2f", get_aabb_area(sponza_geo.m_aabb));
            glm::vec3 dim = sponza_geo.m_aabb.max - sponza_geo.m_aabb.min;
            ImGui::Text("Level Bounding Volume Dimensions %.2f,%.2f,%.2f", dim.x, dim.y, dim.z);
            ImGui::Separator();
            ImGui::Text("Lights");
            ImGui::ColorEdit3("Dir Light Colour", &dir.colour[0]);
            ImGui::DragFloat3("Dir Light Rotation", &dir.direction[0], 1.0f, 0.0f, 360.0f);
            ImGui::DragFloat("Dir Light Intensity", &dir.intensity, 1.0f, 0.0f, 1000.0f);

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
        engine::engine_post_frame();
    }
    engine::engine_shut_down();

    return 0;
}