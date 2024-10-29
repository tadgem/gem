#include <iostream>
#include "imgui.h"

#include "backend.h"
#include "engine.h"
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


void on_im3d(gl_renderer& renderer, scene& current_scene, camera& cam)
{
    if (!current_scene.does_entity_exist((u32)renderer.m_last_selected_entity))
    {
        return;
    }

    if (!current_scene.m_registry.any_of<mesh>(renderer.m_last_selected_entity))
    {
        return;
    }
    mesh& meshc = current_scene.m_registry.get<mesh>(renderer.m_last_selected_entity);
    transform& trans = current_scene.m_registry.get<transform>(renderer.m_last_selected_entity);

    Im3d::DrawAlignedBox(ToIm3D(meshc.m_transformed_aabb.min), ToIm3D(meshc.m_transformed_aabb.max));
}

float get_aabb_area(aabb& bb)
{
    return glm::length(bb.max - bb.min) ;
}

int main()
{
    gl_backend::init(backend_init{ {1920, 1080}, true });
    engine::init();
    asset_manager am{};
    gl_renderer renderer{};

    renderer.init(am);

    custom_orientation = glm::vec3(0, 1, 0);
    camera cam{};
    debug_camera_controller controller{};
    scene* s = new scene("test_scene");
    entity e = s->create_entity("Daddalus");

    e.has_component<entity_data>();
    entity_data& data = e.get_component<entity_data>();
    material mat(renderer.m_gbuffer_shader->m_data);
    e.add_component<material>(renderer.m_gbuffer_shader->m_data);

    // model sponza_geo = model::load_model_and_textures_from_path("assets/models/sponza/Sponza.gltf");
    am.load_asset("assets/models/sponza/Sponza.gltf", asset_type::model, [s, &am, &renderer](asset* a) {
        spdlog::info("adding model to scene");
        model_asset* ma = static_cast<model_asset*>(a);
        ma->m_data.update_aabb();
        s->create_entity_from_model(ma->m_data, renderer.m_gbuffer_shader->m_data, glm::vec3(0.03), glm::vec3(0.0, 0.0, 0.0),
            {
                {"u_diffuse_map", texture_map_type::diffuse},
                {"u_normal_map", texture_map_type::normal},
                {"u_metallic_map", texture_map_type::metallicness},
                {"u_roughness_map", texture_map_type::roughness},
                {"u_ao_map", texture_map_type::ao}
            });
    });

    dir_light dir
    {
        {90.01f, 0.0f, 0.0f},
        {1.0f,1.0f,1.0f},
        2.75f
    };

    entity dir_light_entity = s->create_entity("dir light");
    dir_light& dir2 = e.add_component<dir_light>(dir);
    std::vector<point_light> lights;
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

    std::vector<scene*> scenes{ s };

    while (!gl_backend::s_quit)
    {
        glEnable(GL_DEPTH_TEST);
        am.update();
        
        gl_backend::process_sdl_event();
        gl_backend::engine_pre_frame();      
        glm::vec2 window_dim = gl_backend::get_window_dim();
        renderer.pre_frame(cam);
        controller.update(window_dim, cam);
        cam.update(window_dim);

        for (auto* current_scene : scenes)
        {
            current_scene->on_update();
            on_im3d(renderer, *s, cam);
        }

        glm::vec2 mouse_pos = input::get_mouse_position();
        if (input::get_mouse_button(mouse_button::left) && !ImGui::GetIO().WantCaptureMouse)
        {
            renderer.get_mouse_entity(mouse_pos);
        }

        if (s->does_entity_exist((u32) renderer.m_last_selected_entity))
        {
            entity_data& data = s->m_registry.get<entity_data>(renderer.m_last_selected_entity);
            ImGui::Begin(data.m_name.c_str());
            // do each component ImGui
            ImGui::End();
        }

        {
            renderer.on_imgui(am);

            ImGui::Begin("Demo Settings");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / gl_backend::s_imgui_io->Framerate, gl_backend::s_imgui_io->Framerate);
            ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
            ImGui::Text("Selected Entity ID : %d", renderer.m_last_selected_entity);
            ImGui::Separator();
            ImGui::Text("Lights");
            ImGui::ColorEdit3("Dir Light Colour", &dir2.colour[0]);
            ImGui::DragFloat3("Dir Light Rotation", &dir2.direction[0], 1.0f, 0.0f, 360.0f);
            ImGui::DragFloat("Dir Light Intensity", &dir2.intensity, 1.0f, 0.0f, 1000.0f);

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

        renderer.render(am, cam, scenes);
        gl_backend::engine_post_frame();
    }
    gl_backend::engine_shut_down();
    engine::shutdown();

    return 0;
}