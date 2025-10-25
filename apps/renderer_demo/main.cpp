#include "gem/gem.h"
#include "gem/gl/gl_mesh.h"
#include <sstream>

using namespace nlohmann;
using namespace gem;

void on_im3d(GLRenderer & renderer, Scene & current_scene)
{
    Im3d::DrawAlignedBox(Im3d::Vec3(0.0f), Im3d::Vec3(1.0f));
    if (!current_scene.DoesEntityExist((u32)renderer.m_last_selected_entity))
    {
        return;
    }

    if (!current_scene.registry.any_of<GLMesh>(renderer.m_last_selected_entity))
    {
        return;
    }
    GLMesh & meshc = current_scene.registry.get<GLMesh>(renderer.m_last_selected_entity);
    Im3d::DrawAlignedBox(ToIm3D(meshc.transformed_aabb.min), ToIm3D(meshc.transformed_aabb.max));
}

void on_imgui(GLRenderer & renderer, Scene * s, glm::vec2 mouse_pos,
              DirectionalLight & dir2, Transform & cube_trans, std::vector<PointLight>& lights)
{
    if (s->DoesEntityExist((u32) renderer.m_last_selected_entity))
    {
        EntityData & data = s->registry.get<EntityData>(renderer.m_last_selected_entity);
        ImGui::Begin(data.m_name.c_str());
        // do each component ImGui
        ImGui::End();
    }
    {
        renderer.OnImGui(Engine::assets);

        ImGui::Begin("Demo Settings");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / GPUBackend::Selected()->imgui_io->Framerate,
                    GPUBackend::Selected()->imgui_io->Framerate);

        ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
        ImGui::Text("Selected Entity ID : %d", renderer.m_last_selected_entity);
        ImGui::Separator();
        ImGui::Text("Debug Cube");
        ImGui::DragFloat3("Cube Position", &cube_trans.position[0], 1.0f);
        ImGui::DragFloat3("Cube Euler", &cube_trans.euler[0], 1.0f, 0.0, 360.0f);
        ImGui::DragFloat3("Cube Scale", &cube_trans.scale[0], 1.0f);
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
    Engine::Init(resolution);
    GLRenderer renderer{};
    renderer.Init(Engine::assets, resolution);

    Camera cam{};
    DebugCameraController controller{};
    Scene * s = Engine::scenes.CreateScene("test_scene");
    Entity e = s->CreateEntity("Daddalus");

    e.HasComponent<EntityData>();
    auto& data = e.GetComponent<EntityData>();
    Material mat(renderer.m_gbuffer_shader->handle, renderer.m_gbuffer_shader->m_data);
    e.AddComponent<Material>(renderer.m_gbuffer_shader->handle, renderer.m_gbuffer_shader->m_data);

    Engine::assets.LoadAsset("assets/models/sponza/Sponza.gltf", AssetType::kModel, [s, &renderer](Asset * a) {
        spdlog::info("adding model to scene");
        auto* ma = dynamic_cast<ModelAsset *>(a);
        ma->m_data.UpdateAABB();
        s->CreateEntityFromModel(ma->handle, ma->m_data, renderer.m_gbuffer_shader->handle, renderer.m_gbuffer_shader->m_data, glm::vec3(0.1), glm::vec3(0.0, 0.0, 0.0),
            {
                {"u_diffuse_map", TextureMapType::kDiffuse},
                {"u_normal_map", TextureMapType::kNormal},
                {"u_metallic_map", TextureMapType::kMetallic},
                {"u_roughness_map", TextureMapType::kRoughness},
                {"u_ao_map", TextureMapType::kAO}
            });
        renderer.m_voxel_data.current_bounding_box = ma->m_data.m_aabb;
        nlohmann::json scene_json = Engine::scenes.SaveScene(s);
        std::string scene_json_str = scene_json.dump();
        spdlog::info("finished adding model to scene, dumping scene json");
        spdlog::info(scene_json_str);
    });

    auto cube_entity = s->CreateEntity("Test Cube");
    auto& cube_trans = cube_entity.AddComponent<Transform>();
    auto& cube_mat = cube_entity.AddComponent<Material>(
        renderer.m_gbuffer_textureless_shader->handle,
                renderer.m_gbuffer_textureless_shader->m_data);
    cube_mat.SetUniformValue("u_diffuse_map", glm::vec3(1.0, 0.0, 0.0));
    cube_mat.SetUniformValue("u_metallic_map", 0.0f);
    cube_mat.SetUniformValue("u_roughness_map", 0.0f);
    cube_entity.AddComponent<MeshComponent>(
        MeshComponent{Shapes::kTorusMesh, {}, 0});


    std::vector<PointLight> lights;
    DirectionalLight dir
    {
        {90.01f, 0.0f, 0.0f},
        {1.0f,1.0f,1.0f},
        2.75f
    };
    auto& dir2 = e.AddComponent<DirectionalLight>(dir);
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

    std::vector<Scene *> scenes{ s };

    while (!GPUBackend::Selected()->quit)
    {
        glEnable(GL_DEPTH_TEST);
        Engine::Update();

        GPUBackend::Selected()->ProcessEvents();
        GPUBackend::Selected()->PreFrame();
        glm::vec2 window_dim = GPUBackend::Selected()->GetWindowDimensions();
        renderer.PreFrame(cam);
        controller.Update(window_dim, cam);
        cam.Update(window_dim);

        for (auto* current_scene : scenes)
        {
            current_scene->Update();
            on_im3d(renderer, *s);
        }

        glm::vec2 mouse_pos = Input::GetMousePosition();
        if (Input::GetMouseButton(MouseButton::kLeft) && !ImGui::GetIO().WantCaptureMouse)
        {
            renderer.GetEntityAtScreenPosition(mouse_pos);
        }

        on_imgui(renderer, s, mouse_pos, dir2, cube_trans, lights);

        renderer.Render(Engine::assets, cam, scenes);
        GPUBackend::Selected()->PostFrame();
    }
    GPUBackend::Selected()->ShutDown();
    Engine::Shutdown();

    return 0;
}