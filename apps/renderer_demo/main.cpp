#include "gem/gem.h"
#include <sstream>

using namespace nlohmann;
using namespace gem;

void on_im3d(gl_renderer& renderer, scene& current_scene)
{
    Im3d::DrawAlignedBox(ToIm3D(renderer.m_voxel_data.current_bounding_box.min), ToIm3D(renderer.m_voxel_data.current_bounding_box.max));
    if (!current_scene.does_entity_exist((u32)renderer.m_last_selected_entity))
    {
        return;
    }

    if (!current_scene.m_registry.any_of<mesh>(renderer.m_last_selected_entity))
    {
        return;
    }
    mesh& meshc = current_scene.m_registry.get<mesh>(renderer.m_last_selected_entity);
    Im3d::DrawAlignedBox(ToIm3D(meshc.m_transformed_aabb.min), ToIm3D(meshc.m_transformed_aabb.max));
}

void on_imgui(gl_renderer& renderer, scene* s, glm::vec2 mouse_pos, dir_light& dir2, transform& cube_trans, std::vector<point_light>& lights)
{
    if (s->does_entity_exist((u32) renderer.m_last_selected_entity))
    {
        entity_data& data = s->m_registry.get<entity_data>(renderer.m_last_selected_entity);
        ImGui::Begin(data.m_name.c_str());
        // do each component ImGui
        ImGui::End();
    }
    {
        renderer.on_imgui(engine::assets);

        ImGui::Begin("Demo Settings");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / gpu_backend::selected()->m_imgui_io->Framerate,
                    gpu_backend::selected()->m_imgui_io->Framerate);

        ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
        ImGui::Text("Selected Entity ID : %d", renderer.m_last_selected_entity);
        ImGui::Separator();
        ImGui::Text("Debug Cube");
        ImGui::DragFloat3("Cube Position", &cube_trans.m_position[0], 1.0f);
        ImGui::DragFloat3("Cube Euler", &cube_trans.m_euler[0], 1.0f, 0.0, 360.0f);
        ImGui::DragFloat3("Cube Scale", &cube_trans.m_scale[0], 1.0f);
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
}

int main()
{
    glm::ivec2 resolution = {1920, 1080};
    engine::init();
    gl_renderer renderer{};
    renderer.init(engine::assets, resolution);

    camera cam{};
    debug_camera_controller controller{};
    scene* s = engine::scenes.create_scene("test_scene");
    entity e = s->create_entity("Daddalus");

    e.has_component<entity_data>();
    auto& data = e.get_component<entity_data>();
    material mat(renderer.m_gbuffer_shader->m_handle, renderer.m_gbuffer_shader->m_data);
    e.add_component<material>(renderer.m_gbuffer_shader->m_handle, renderer.m_gbuffer_shader->m_data);

    engine::assets.load_asset("assets/models/sponza/Sponza.gltf", asset_type::model, [s, &renderer](asset* a) {
        spdlog::info("adding model to scene");
        auto* ma = dynamic_cast<model_asset*>(a);
        ma->m_data.update_aabb();
        s->create_entity_from_model(ma->m_handle, ma->m_data, renderer.m_gbuffer_shader->m_handle, renderer.m_gbuffer_shader->m_data, glm::vec3(0.1), glm::vec3(0.0, 0.0, 0.0),
            {
                {"u_diffuse_map", texture_map_type::diffuse},
                {"u_normal_map", texture_map_type::normal},
                {"u_metallic_map", texture_map_type::metallicness},
                {"u_roughness_map", texture_map_type::roughness},
                {"u_ao_map", texture_map_type::ao}
            });

        nlohmann::json scene_json = engine::scenes.save_scene(s);
        std::string scene_json_str = scene_json.dump();
        spdlog::info("finished adding model to scene, dumping scene json");
        spdlog::info(scene_json_str);
    });

    auto cube_entity = s->create_entity("Test Cube");
    auto& cube_trans = cube_entity.add_component<transform>();
    auto& cube_mat = cube_entity.add_component<material>(
        renderer.m_gbuffer_textureless_shader->m_handle,
                renderer.m_gbuffer_textureless_shader->m_data);
    cube_mat.set_uniform_value("u_diffuse_map", glm::vec3(1.0, 0.0, 0.0));
    cube_mat.set_uniform_value("u_metallic_map", 0.0f);
    cube_mat.set_uniform_value("u_roughness_map", 0.0f);
    cube_entity.add_component<mesh_component>(
        mesh_component {shapes::s_torus_mesh, {}, 0});


    std::vector<point_light> lights;
    dir_light dir
    {
        {90.01f, 0.0f, 0.0f},
        {1.0f,1.0f,1.0f},
        2.75f
    };
    auto& dir2 = e.add_component<dir_light>(dir);
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

    std::vector<scene*> scenes{ s };

    while (!gpu_backend::selected()->m_quit)
    {
        glEnable(GL_DEPTH_TEST);
        engine::update();
        
        gpu_backend::selected()->process_sdl_event();
        gpu_backend::selected()->engine_pre_frame();
        glm::vec2 window_dim = gpu_backend::selected()->get_window_dim();
        renderer.pre_frame(cam);
        controller.update(window_dim, cam);
        cam.update(window_dim);

        for (auto* current_scene : scenes)
        {
            current_scene->on_update();
            on_im3d(renderer, *s);
        }

        glm::vec2 mouse_pos = input::get_mouse_position();
        if (input::get_mouse_button(mouse_button::left) && !ImGui::GetIO().WantCaptureMouse)
        {
            renderer.get_mouse_entity(mouse_pos);
        }

        on_imgui(renderer, s, mouse_pos, dir2, cube_trans, lights);

        renderer.render(engine::assets, cam, scenes);
        gpu_backend::selected()->engine_post_frame();
    }
    gpu_backend::selected()->engine_shut_down();
    engine::shutdown();

    return 0;
}