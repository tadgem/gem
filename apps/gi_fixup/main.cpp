#undef SDL_THREAD_WINDOWS
#include "gem/gem.h"
#include <sstream>
#include "imgui_impl_opengl3.h"
#include "gem/profile.h"
#include "gem/utils.h"

using namespace nlohmann;
using namespace gem;

struct DebugVoxelGrid
{
    VAO     m_vao;
    AABB    m_transformed_aabb;
    int     m_square_resolution;
};

void on_imgui(GLRenderer & renderer, Scene * s, glm::vec2 mouse_pos,
              DirectionalLight & dir2, Transform & cube_trans, std::vector<PointLight>& lights)
{
    if (s->DoesEntityExist((u32) renderer.last_selected_entity))
    {
        EntityData & data = s->registry.get<EntityData>(renderer.last_selected_entity);
        ImGui::Begin(data.entity_name.c_str());
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
        ImGui::Text("Selected Entity ID : %d", renderer.last_selected_entity);
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

DebugVoxelGrid create_debug_voxel_renderer(GLShader& viz_shader, const AABB& initial_aabb, int square_resolution)
{
    glm::mat4 model = Utils::GetModelMatrix (glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.2f));
    AABB transformed_aabb = Utils::TransformAABB(initial_aabb, model);
    int cubed_res = pow(square_resolution, 3);

    glm::vec3 aabb_dim = transformed_aabb.max - transformed_aabb.min;
    glm::vec3 unit = glm::vec3((aabb_dim.x / cubed_res), (aabb_dim.y / cubed_res), (aabb_dim.z / cubed_res));

    std::vector<glm::mat4> instance_matrices;
    std::vector<glm::vec3> instance_uvs;


    for (auto i = 0; i < cubed_res; i++)
    {
        //create a new VAO for debug cubes
        // first vbo is same as cube
        // instance vbo is per-instance transform

        float z = transformed_aabb.min.z;
        float y = transformed_aabb.min.y;
        float x = transformed_aabb.min.x;

        float z_offset = i / (cubed_res * cubed_res);
        float y_offset = (i / cubed_res) % cubed_res;
        float x_offset = i % cubed_res;

        float z_offset2 = z_offset * unit.z;
        float y_offset2 = y_offset * unit.y;
        float x_offset2 = x_offset * unit.x;

        z += z_offset2;
        y += y_offset2;
        x += x_offset2;

        instance_uvs.push_back({ (x_offset + 1) / cubed_res ,(y_offset + 1) / cubed_res,(z_offset + 1) / cubed_res });
        instance_matrices.push_back(Utils::GetModelMatrix({ x,y,z }, { 0,90,0 }, unit));
    }

    VAO instanced_cubes = Shapes::GenerateInstancedCube(instance_matrices, instance_uvs);

    return DebugVoxelGrid {
        instanced_cubes, transformed_aabb, square_resolution
    };

}

int main()
{
    glm::ivec2 resolution = {1600, 900};
    Engine::Init(resolution);
    GLRenderer renderer{};
    renderer.Init(Engine::assets, resolution);

    Camera cam{};
    DebugCameraController controller{};
    Scene * s = Engine::scenes.CreateScene("test_scene");
    Entity e = s->CreateEntity("Daddalus");

    e.HasComponent<EntityData>();
    auto& data = e.GetComponent<EntityData>();
    Material mat(renderer.gbuffer_shader->handle, renderer.gbuffer_shader->data);
    e.AddComponent<Material>(renderer.gbuffer_shader->handle, renderer.gbuffer_shader->data);

    Engine::assets.LoadAsset("assets/models/sponza/Sponza.gltf", AssetType::kModel, [s, &renderer](Asset * a) {
        spdlog::info("adding model to scene");
        auto* ma = dynamic_cast<ModelAsset *>(a);
        ma->data.UpdateAABB();
        s->CreateEntityFromModel(ma->handle, ma->data, renderer.gbuffer_shader->handle, renderer.gbuffer_shader->data, glm::vec3(0.1), glm::vec3(0.0, 0.0, 0.0),
            {
                {"u_diffuse_map",   TextureMapType::kDiffuse},
                {"u_normal_map",    TextureMapType::kNormal},
                {"u_metallic_map",  TextureMapType::kMetallic},
                {"u_roughness_map", TextureMapType::kRoughness},
                {"u_ao_map",        TextureMapType::kAO}
            });
        glm::mat4 model = Utils::GetModelMatrix(glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.1));
        renderer.voxel_data.current_bounding_box = Utils::TransformAABB(ma->data.aabb, model);
    });

    auto cube_entity = s->CreateEntity("Test Cube");
    auto& cube_trans = cube_entity.AddComponent<Transform>();
    auto& cube_mat = cube_entity.AddComponent<Material>(
        renderer.gbuffer_textureless_shader->handle,
                renderer.gbuffer_textureless_shader->data);
    cube_mat.SetUniformValue("u_diffuse", glm::vec3(1.0, 0.0, 0.0));
    cube_mat.SetUniformValue("u_metallic", 0.0f);
    cube_mat.SetUniformValue("u_roughness", 0.0f);
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
