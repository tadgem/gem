#include "gem/gem.h"
#include <sstream>
#include "imgui_impl_opengl3.h"
#include "gem/profile.h"

using namespace nlohmann;
using namespace gem;

static std::array<Im3d::Color, 4> s_colours
{
        Im3d::Color_Red,
        Im3d::Color_Magenta,
        Im3d::Color_Yellow,
        Im3d::Color_Green
};

template<size_t _NumSlices>
struct vxgi_data_n
{

  enum AlongAxis : int
  {
    X = 0,
    Y = 1,
    Z = 2
  };

  Texture                                 m_voxel_texture; // 3D Texture (Voxel Data)
  glm::ivec3                              m_resolution;
  glm::vec3                               m_voxel_unit; // scale of each texel
  glm::vec3                               m_aabb_dim{200.0, 100.0, 200.0};
  glm::vec3                               m_center_pos{0.0, 0.0, 0.0};
  AABB                                    m_bounding_volume;
  std::array<glm::mat4, _NumSlices>       m_slice_vp_matrices;
  std::array<uint32_t, _NumSlices>        m_current_slice_indices;
  std::array<GLFramebuffer, _NumSlices>   m_slice_renders;
  AlongAxis                               m_axis = AlongAxis::Y;
  glm::mat4                               m_debug_vp;
  glm::vec3                               m_debug_eye;

  void update_bounding_volume(const glm::vec3& camera_pos)
  {
    ZoneScoped;
    m_voxel_unit = m_aabb_dim / glm::vec3(m_resolution);
    m_center_pos = camera_pos;
    glm::vec3 half_dim = m_aabb_dim * 0.5f;
    m_bounding_volume.m_min = camera_pos - half_dim;
    m_bounding_volume.m_max = camera_pos + half_dim;

    for(size_t n = 0; n < _NumSlices; n++)
    {
      glm::mat4 projection (1.0);
      glm::mat4 view (1.0);
      glm::vec3 center, eye;
      m_current_slice_indices[n] += _NumSlices;
      m_current_slice_indices[n] = m_current_slice_indices[n] >= m_resolution[m_axis]
                                       ? n : m_current_slice_indices[n];
      float back = m_bounding_volume.m_min[m_axis]
                  + (m_voxel_unit[m_axis] * m_current_slice_indices[n]);

      float front = m_bounding_volume.m_min[m_axis]
                  + (m_voxel_unit[m_axis] * (m_current_slice_indices[n] + 4));
      switch(m_axis)
      {
      case AlongAxis::X:
        projection = glm::ortho(
            -half_dim.z,
            half_dim.z,
            -half_dim.y,
            half_dim.y,
            back,
            front
            );

        center = glm::vec3 (m_bounding_volume.m_min.x - half_dim.x, m_center_pos.y + FLT_EPSILON, m_center_pos.z + FLT_EPSILON);
        eye = glm::vec3 (m_bounding_volume.m_max.x - half_dim.x,m_center_pos.y, m_center_pos.z);
        break;
      case AlongAxis::Y:
        projection = glm::ortho(
            -half_dim.x,
            half_dim.x,
            -half_dim.z,
            half_dim.z, back, front);

        center = glm::vec3 (m_center_pos.x + FLT_EPSILON, m_bounding_volume.m_min.y - half_dim.y, m_center_pos.z + FLT_EPSILON);
        eye = glm::vec3 (m_center_pos.x, m_bounding_volume.m_max.y - half_dim.y, m_center_pos.z);
        break;
      case AlongAxis::Z:
        break;
      }
      view = glm::lookAt(eye, center, glm::vec3(1.0, 0.0, 0.0));
      //glm::mat4 view = glm::translate(glm::mat4(1.0), eye);
      m_slice_vp_matrices[n] =  projection * view;

    }

  }

  void render(gem::AssetManager& am,
              std::vector<gem::Scene*>& scenes,
              gem::Camera& cam,
              GLShader& forward_lighting_shader,
              GLFramebuffer &dir_light_shadow_buffer,
              glm::ivec2 window_res)
  {
    ZoneScoped;
    GEM_GPU_MARKER("VXGI Lighting Slice Pass");
    glLineWidth(10.0f);
    DirectionalLight sun{};
    std::vector<PointLight> point_lights{};
    if (!scenes.empty()) {
      auto dir_light_view = scenes.front()->m_registry.view<DirectionalLight>();
      for (auto [e, dir_light_c] : dir_light_view.each()) {
        sun = dir_light_c;
        break;
      }
    }

    glViewport(0,0, 256,256);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    forward_lighting_shader.use();

    // set sampler indices
    forward_lighting_shader.set_int("u_diffuse_map", 0);
    forward_lighting_shader.set_int("u_normal_map", 1);
    forward_lighting_shader.set_int("u_metallic_map", 2);
    forward_lighting_shader.set_int("u_roughness_map", 3);
    forward_lighting_shader.set_int("u_ao_map", 4);
    forward_lighting_shader.set_int("u_dir_light_shadow_map", 5);

    glm::vec3 dir =
        glm::quat(glm::radians(sun.direction)) * glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.0) - (dir * 100.0f);

    forward_lighting_shader.set_vec3("u_dir_light_pos", lightPos);

    forward_lighting_shader.set_vec3("u_dir_light.direction",
                             Utils::get_forward(sun.direction));
    forward_lighting_shader.set_vec3("u_dir_light.colour", sun.colour);
    forward_lighting_shader.set_mat4("u_dir_light.light_space_matrix",
                             sun.light_space_matrix);
    forward_lighting_shader.set_float("u_dir_light.intensity", sun.intensity);

    int num_point_lights = std::min((int)point_lights.size(), 16);

    for (int i = 0; i < num_point_lights; i++) {
      std::stringstream pos_name;
      pos_name << "u_point_lights[" << i << "].position";
      std::stringstream col_name;
      col_name << "u_point_lights[" << i << "].colour";
      std::stringstream rad_name;
      rad_name << "u_point_lights[" << i << "].radius";
      std::stringstream int_name;
      int_name << "u_point_lights[" << i << "].intensity";

      forward_lighting_shader.set_vec3(pos_name.str(), point_lights[i].position);
      forward_lighting_shader.set_vec3(col_name.str(), point_lights[i].colour);
      forward_lighting_shader.set_float(rad_name.str(), point_lights[i].radius);
      forward_lighting_shader.set_float(int_name.str(), point_lights[i].intensity);
    }
    // set dirlight shadow map
    Texture::bind_sampler_handle(dir_light_shadow_buffer.m_depth_attachment,
                                 GL_TEXTURE5);

    for(auto n = 0; n < _NumSlices; n++)
    {
      m_slice_renders[n].bind();

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      forward_lighting_shader.set_mat4("u_vp", m_slice_vp_matrices[n]);
      forward_lighting_shader.set_vec3("u_cam_pos", cam.m_pos);

      for(auto* s : scenes)
      {

        auto scene_renderables = s->m_registry.view<Transform, MeshComponent, Material>();
        for (auto [e, trans, emesh, ematerial] : scene_renderables.each()) {
          // TODO:
          // this wont work, we need the samplers and material values
          // but this pass uses a different shader from the gbuffer pass.
          // in this particular case, the forward shader mirrors the gbuffer pass
          // so we can likely just fudge this slightly and push the same values to
          // the forward pass shader.

          if(ematerial.m_uniform_values.find("u_diffuse_map") != ematerial.m_uniform_values.end()) {
            auto sampler = std::any_cast<SamplerInfo>(
                ematerial.m_uniform_values["u_diffuse_map"]);

            if(sampler.tex_entry.m_texture) {
              Texture::bind_sampler_handle(
                  sampler.tex_entry.m_texture->m_handle, GL_TEXTURE0);
            }
          }

          if(ematerial.m_uniform_values.find("u_normal_map") != ematerial.m_uniform_values.end()) {
            auto sampler = std::any_cast<SamplerInfo>(
                ematerial.m_uniform_values["u_normal_map"]);

            if(sampler.tex_entry.m_texture) {
              Texture::bind_sampler_handle(
                  sampler.tex_entry.m_texture->m_handle, GL_TEXTURE1);
            }
          }

          if(ematerial.m_uniform_values.find("u_metallic_map") != ematerial.m_uniform_values.end()) {
            auto sampler = std::any_cast<SamplerInfo>(
                ematerial.m_uniform_values["u_metallic_map"]);

            if(sampler.tex_entry.m_texture) {
              Texture::bind_sampler_handle(
                  sampler.tex_entry.m_texture->m_handle, GL_TEXTURE2);
            }
          }

          if(ematerial.m_uniform_values.find("u_roughness_map") != ematerial.m_uniform_values.end()) {
            auto sampler = std::any_cast<SamplerInfo>(
                ematerial.m_uniform_values["u_roughness_map"]);

            if(sampler.tex_entry.m_texture) {
              Texture::bind_sampler_handle(
                  sampler.tex_entry.m_texture->m_handle, GL_TEXTURE3);
            }
          }

          if(ematerial.m_uniform_values.find("u_ao_map") != ematerial.m_uniform_values.end()) {
            auto sampler = std::any_cast<SamplerInfo>(
                ematerial.m_uniform_values["u_ao_map"]);

            if(sampler.tex_entry.m_texture) {
              Texture::bind_sampler_handle(
                  sampler.tex_entry.m_texture->m_handle, GL_TEXTURE4);
            }
          }

          forward_lighting_shader.set_mat4("u_model", trans.m_model);
          forward_lighting_shader.set_mat4("u_normal", trans.m_normal_matrix);
          emesh.m_mesh.draw();

          Texture::bind_sampler_handle(0, GL_TEXTURE0);
          Texture::bind_sampler_handle(0, GL_TEXTURE1);
          Texture::bind_sampler_handle(0, GL_TEXTURE2);
          Texture::bind_sampler_handle(0, GL_TEXTURE3);
          Texture::bind_sampler_handle(0, GL_TEXTURE4);
        }
      }
      m_slice_renders[n].unbind();
    }
    glViewport(0,0,window_res.x, window_res.y);
    glLineWidth(1.0f);  }

  void on_imgui()
  {
    ImGui::Begin("VXGI V2 Debug");

    if(ImGui::BeginMenu("Axis")) {
      if (ImGui::MenuItem("X")) {
        m_axis = AlongAxis::X;
      }
      if (ImGui::MenuItem("Y")) {
        m_axis = AlongAxis::Y;
      }
      if (ImGui::MenuItem("Z")) {
        m_axis = AlongAxis::Z;
      }

      ImGui::EndMenu();
    }
    ImGui::Image((ImTextureID)m_slice_renders[0].m_colour_attachments[0], {256, 256});
    ImGui::SameLine();
    ImGui::Image((ImTextureID)m_slice_renders[1].m_colour_attachments[0], {256, 256});
    ImGui::Image((ImTextureID)m_slice_renders[2].m_colour_attachments[0], {256, 256});
    ImGui::SameLine();
    ImGui::Image((ImTextureID)m_slice_renders[3].m_colour_attachments[0], {256, 256});
    ImGui::End();

  }

  explicit vxgi_data_n(glm::ivec3 res)
  {
    m_resolution = res;
    for(size_t n = 0; n < _NumSlices; n++) {
      m_current_slice_indices[n] = n;
      GLFramebuffer slice_fb = GLFramebuffer::create({m_resolution.x, m_resolution.z},
      {
          {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
      }, true);
      m_slice_renders[n] = slice_fb;
    }
    update_bounding_volume(glm::vec3(0.0));
  }

};

using VXGIData = vxgi_data_n<4>;

void on_im3d(GLRenderer & renderer, Scene & current_scene, VXGIData & vxgi,
             Camera & cam, glm::mat4& dir_light_matrix)
{

    static float left = -50.0f;
    static float right = 50.0f;
    static float bottom = -50.0f;
    static float top = 50.0f;
    static float near0 = 0.0f;
    static float far0 = 50.0f;
    static glm::vec3 center = glm::vec3(FLT_EPSILON);
    static glm::vec3 eye = glm::vec3(0.0, 50.0, 0.0);

    vxgi.update_bounding_volume(glm::vec3(0.0f));

    DebugDraw::DrawBoundingBox(vxgi.m_bounding_volume.m_min, vxgi.m_bounding_volume.m_max, 2.0f, Im3d::Color_Pink);
    for(auto n = 0; n < 4; n++)
    {
      DebugDraw::DrawFrustum(vxgi.m_slice_vp_matrices[n], 1.0, s_colours[n]);
    }

    glm::mat4 projection = glm::ortho(
        left,
        right,
        bottom,
        top,
        near0,
        far0);

    glm::mat4 view = glm::lookAt(eye, center, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 vp = projection * view;
    vxgi.m_debug_vp = vp;
    vxgi.m_debug_eye = eye;

    DebugDraw::DrawFrustum(dir_light_matrix, 2.0f, Im3d::Color_Cyan);
    DebugDraw::DrawFrustum(vp, 2.0, Im3d::Color_Purple);
    DebugDraw::DrawLine(eye, center, 2.0, Im3d::Color_Green);
    Im3d::Text(ToIm3D(eye), Im3d::TextFlags_Default, "Eye Pos");
    Im3d::Text(ToIm3D(center), Im3d::TextFlags_Default, "Center Pos");

    ImGui::Begin("Debug Ortho Projections");
    ImGui::DragFloat("Left", &left);
    ImGui::DragFloat("Right", &right);
    ImGui::DragFloat("Bottom", &bottom);
    ImGui::DragFloat("Top", &top);
    ImGui::DragFloat("Near", &near0);
    ImGui::DragFloat("Far", &far0);
    ImGui::DragFloat3("Eye", &eye[0]);
    ImGui::DragFloat3("Center", &center[0]);
    ImGui::End();

}

void on_imgui(GLRenderer & renderer, Scene * s, glm::vec2 mouse_pos,
              DirectionalLight & dir2, Transform & cube_trans, std::vector<PointLight>& lights)
{
    if (s->does_entity_exist((u32) renderer.m_last_selected_entity))
    {
        EntityData & data = s->m_registry.get<EntityData>(renderer.m_last_selected_entity);
        ImGui::Begin(data.m_name.c_str());
        // do each component ImGui
        ImGui::End();
    }
    {
        renderer.on_imgui(Engine::assets);

        ImGui::Begin("Demo Settings");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / GPUBackend::selected()->m_imgui_io->Framerate,
                    GPUBackend::selected()->m_imgui_io->Framerate);

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
    Engine::init();
    GLRenderer renderer{};
    renderer.init(Engine::assets, resolution);

    Camera cam{};
    DebugCameraController controller{};
    Scene * s = Engine::scenes.create_scene("test_scene");
    Entity e = s->create_entity("Daddalus");

    e.has_component<EntityData>();
    auto& data = e.get_component<EntityData>();
    Material mat(renderer.m_gbuffer_shader->m_handle, renderer.m_gbuffer_shader->m_data);
    e.add_component<Material>(renderer.m_gbuffer_shader->m_handle, renderer.m_gbuffer_shader->m_data);

    Engine::assets.load_asset("assets/models/sponza/Sponza.gltf", AssetType::model, [s, &renderer](Asset * a) {
        spdlog::info("adding model to scene");
        auto* ma = dynamic_cast<ModelAsset *>(a);
        ma->m_data.update_aabb();
        s->create_entity_from_model(ma->m_handle, ma->m_data, renderer.m_gbuffer_shader->m_handle, renderer.m_gbuffer_shader->m_data, glm::vec3(0.1), glm::vec3(0.0, 0.0, 0.0),
            {
                {"u_diffuse_map",   TextureMapType::diffuse},
                {"u_normal_map",    TextureMapType::normal},
                {"u_metallic_map",  TextureMapType::metallicness},
                {"u_roughness_map", TextureMapType::roughness},
                {"u_ao_map",        TextureMapType::ao}
            });

        nlohmann::json scene_json = Engine::scenes.save_scene(s);
        std::string scene_json_str = scene_json.dump();
        spdlog::info("finished adding model to scene, dumping scene json");
        spdlog::info(scene_json_str);
    });

    auto cube_entity = s->create_entity("Test Cube");
    auto& cube_trans = cube_entity.add_component<Transform>();
    auto& cube_mat = cube_entity.add_component<Material>(
        renderer.m_gbuffer_textureless_shader->m_handle,
                renderer.m_gbuffer_textureless_shader->m_data);
    cube_mat.set_uniform_value("u_diffuse", glm::vec3(1.0, 0.0, 0.0));
    cube_mat.set_uniform_value("u_metallic", 0.0f);
    cube_mat.set_uniform_value("u_roughness", 0.0f);
    cube_entity.add_component<MeshComponent>(
        MeshComponent{Shapes::s_torus_mesh, {}, 0});


    std::vector<PointLight> lights;
    DirectionalLight dir
    {
        {90.01f, 0.0f, 0.0f},
        {1.0f,1.0f,1.0f},
        2.75f
    };
    auto& dir2 = e.add_component<DirectionalLight>(dir);
    lights.push_back({ {0.0, 0.0, 0.0}, {255.0, 0.0, 0.0}, 10.0f});
    lights.push_back({ {10.0, 0.0, 10.0}, {255.0, 255.0, 0.0}, 20.0f });
    lights.push_back({ {-10.0, 0.0, -10.0}, {0.0, 255.0, 0.0}, 30.0f });
    lights.push_back({ {-10.0, 0.0, 10.0}, {0.0, 0.0, 255.0} , 40.0f});

    std::vector<Scene *> scenes{ s };
    VXGIData vxgi({256,256,256});
    vxgi.update_bounding_volume(glm::vec3(0.0f));

    while (!GPUBackend::selected()->m_quit)
    {
        glEnable(GL_DEPTH_TEST);
        Engine::update();

        GPUBackend::selected()->process_sdl_event();
        GPUBackend::selected()->engine_pre_frame();
        glm::vec2 window_dim = GPUBackend::selected()->get_window_dim();
        renderer.pre_frame(cam);
        controller.update(window_dim, cam);
        cam.update(window_dim);
        vxgi.update_bounding_volume(cam.m_pos);

        for (auto* current_scene : scenes)
        {
            current_scene->on_update();
            on_im3d(renderer, *s, vxgi, cam, dir2.light_space_matrix);
        }

        glm::vec2 mouse_pos = Input::get_mouse_position();
        if (Input::get_mouse_button(MouseButton::left) && !ImGui::GetIO().WantCaptureMouse)
        {
            renderer.get_mouse_entity(mouse_pos);
        }

        on_imgui(renderer, s, mouse_pos, dir2, cube_trans, lights);


        vxgi.render(Engine::assets, scenes, cam,
                    renderer.m_forward_lighting_shader->m_data,
                    renderer.m_dir_light_shadow_buffer, window_dim);

        vxgi.on_imgui();
        renderer.render(Engine::assets, cam, scenes);
        GPUBackend::selected()->engine_post_frame();
    }
    GPUBackend::selected()->engine_shut_down();
    Engine::shutdown();

    return 0;
}
