#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/gl_renderer.h"
#include "gem/asset_manager.h"
#include "gem/backend.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/tech/gbuffer.h"
#include "gem/gl/tech/lighting.h"
#include "gem/gl/tech/shadow.h"
#include "gem/gl/tech/ssr.h"
#include "gem/gl/tech/taa.h"
#include "gem/gl/tech/tech_utils.h"
#include "gem/gl/tech/vxgi.h"
#include "gem/input.h"
#include "gem/lights.h"
#include "gem/profile.h"
#include "gem/transform.h"
#include "im3d/im3d_math.h"
#include "imgui.h"

namespace gem {

void GLRenderer::init(AssetManager &am, glm::ivec2 resolution) {
  ZoneScoped;
  TracyGpuContext;
  m_frame_index = 0;
  m_im3d_state = GLIm3d::load_im3d();

  am.load_asset("assets/shaders/gbuffer.shader", AssetType::shader);
  am.load_asset("assets/shaders/gbuffer_textureless.shader", AssetType::shader);
  am.load_asset("assets/shaders/lighting.shader", AssetType::shader);
  am.load_asset("assets/shaders/forward_lighting.shader", AssetType::shader);
  am.load_asset("assets/shaders/visualize_3d_tex.shader", AssetType::shader);
  am.load_asset("assets/shaders/visualize_3d_tex_instances.shader",
                AssetType::shader);
  am.load_asset("assets/shaders/present.shader", AssetType::shader);
  am.load_asset("assets/shaders/dir_light_shadow.shader", AssetType::shader);
  am.load_asset("assets/shaders/voxel_cone_tracing.shader", AssetType::shader);
  am.load_asset("assets/shaders/ssr.shader", AssetType::shader);
  am.load_asset("assets/shaders/taa.shader", AssetType::shader);
  am.load_asset("assets/shaders/denoise.shader", AssetType::shader);
  am.load_asset("assets/shaders/gi_combine.shader", AssetType::shader);
  am.load_asset("assets/shaders/downsample.shader", AssetType::shader);
  am.load_asset("assets/shaders/gbuffer_voxelization.shader",
                AssetType::shader);
  am.load_asset("assets/shaders/voxel_mips.shader", AssetType::shader);
  am.load_asset("assets/shaders/voxel_reprojection.shader", AssetType::shader);
  am.load_asset("assets/shaders/voxel_blit.shader", AssetType::shader);
  am.load_asset("assets/shaders/voxel_clear.shader", AssetType::shader);

  am.wait_all_assets();
  m_gbuffer_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/gbuffer.shader");
  m_gbuffer_textureless_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/gbuffer_textureless.shader");
  m_forward_lighting_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/forward_lighting.shader");
  m_lighting_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/lighting.shader");
  m_visualise_3d_tex_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/visualize_3d_tex.shader");
  m_visualise_3d_tex_instances_shader =
      am.get_asset<GLShader, AssetType::shader>(
          "assets/shaders/visualize_3d_tex_instances.shader");
  m_present_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/present.shader");
  m_dir_light_shadow_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/dir_light_shadow.shader");
  m_voxel_cone_tracing_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/voxel_cone_tracing.shader");
  m_ssr_shader =
      am.get_asset<GLShader, AssetType::shader>("assets/shaders/ssr.shader");
  m_taa_shader =
      am.get_asset<GLShader, AssetType::shader>("assets/shaders/taa.shader");
  m_denoise_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/denoise.shader");
  m_combine_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/gi_combine.shader");
  m_downsample_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/downsample.shader");
  m_compute_voxelize_gbuffer_shader =
      am.get_asset<GLShader, AssetType::shader>(
          "assets/shaders/gbuffer_voxelization.shader");
  m_compute_voxel_mips_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/voxel_mips.shader");
  m_compute_voxel_reprojection_shader =
      am.get_asset<GLShader, AssetType::shader>(
          "assets/shaders/voxel_reprojection.shader");
  m_compute_voxel_blit_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/voxel_blit.shader");
  m_compute_voxel_clear_shader = am.get_asset<GLShader, AssetType::shader>(
      "assets/shaders/voxel_clear.shader");

  m_window_resolution = resolution;
  const int shadow_resolution = 4096;
  m_gbuffer =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA32F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGB, GL_RGB16F, GL_NEAREST, GL_FLOAT},
                             },
                             true);

  m_gbuffer_downsample =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_dir_light_shadow_buffer =
      GLFramebuffer::create({shadow_resolution, shadow_resolution}, {}, true);

  m_lightpass_buffer =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_lightpass_buffer_resolve =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_lightpass_buffer_history =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_position_buffer_history =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  glm::vec2 gi_res = {m_window_resolution.x * m_vxgi_resolution_scale,
                      m_window_resolution.y * m_vxgi_resolution_scale};
  m_conetracing_buffer =
      GLFramebuffer::create(gi_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_conetracing_buffer_denoise =
      GLFramebuffer::create(gi_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_conetracing_buffer_resolve =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_conetracing_buffer_history =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  glm::vec2 ssr_res = {m_window_resolution.x * m_ssr_resolution_scale,
                       m_window_resolution.y * m_ssr_resolution_scale};
  m_ssr_buffer =
      GLFramebuffer::create(ssr_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_ssr_buffer_denoise =
      GLFramebuffer::create(ssr_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_ssr_buffer_resolve =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_ssr_buffer_history =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_final_pass =
      GLFramebuffer::create(m_window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA8, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  m_voxel_data = Voxel::create_grid(s_voxel_resolution, AABB{});
  Camera cam{}; // TODO: clean this up, just need a position of 0,0,0 to init
  m_voxel_data.update_voxel_unit();
  m_voxel_visualiser = Voxel::create_grid_visualiser(
      m_voxel_data, m_visualise_3d_tex_shader->m_data,
      m_visualise_3d_tex_instances_shader->m_data, 8);
}

void GLRenderer::pre_frame(Camera &cam) {
  ZoneScoped;

  GLIm3d::new_frame_im3d(m_im3d_state, m_window_resolution, cam);
}

void GLRenderer::render(AssetManager &am, Camera &cam,
                         std::vector<Scene *> &scenes) {
  ZoneScoped;
  FrameMark;

  if(m_debug_simulate_low_framerate)
  {
    SDL_Delay(50);
  }

  if (p_clear_voxel_grid) {
    open_gl::tech::VXGI::dispatch_clear_voxel(
        m_compute_voxel_clear_shader->m_data, m_voxel_data, s_voxel_resolution);
    p_clear_voxel_grid = false;
  }

  m_voxel_data.update_voxel_unit();

  {
    TracyGpuZone("GBuffer Voxelization");
    open_gl::tech::VXGI::dispatch_gbuffer_voxelization(
        m_compute_voxelize_gbuffer_shader->m_data, m_voxel_data, m_gbuffer,
        m_lightpass_buffer, m_window_resolution);
  }

  {
    TracyGpuZone("GBuffer Voxelization MIPS");
    open_gl::tech::VXGI::dispatch_gen_voxel_mips(
        m_compute_voxel_mips_shader->m_data, m_voxel_data, s_voxel_resolution);
  }

  {
    TracyGpuZone("GBuffer");
    m_gbuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    open_gl::tech::GBuffer::dispatch_gbuffer_with_id(
        m_frame_index, m_gbuffer, m_position_buffer_history,
        m_gbuffer_shader->m_data, am, cam, scenes, m_window_resolution);

    open_gl::tech::GBuffer::dispatch_gbuffer_textureless_with_id(
        m_frame_index, m_gbuffer, m_position_buffer_history,
        m_gbuffer_textureless_shader->m_data, am, cam, scenes,
        m_window_resolution);

    m_frame_index++;
  }
  // TODO: Need a way to get a single instance more efficiently
  DirectionalLight dir{};
  std::vector<PointLight> point_lights{};
  if (!scenes.empty()) {
    auto dir_light_view = scenes.front()->m_registry.view<DirectionalLight>();
    for (auto [e, dir_light_c] : dir_light_view.each()) {
      dir = dir_light_c;
      break;
    }
  }
  {
    TracyGpuZone("Dir Light Shadow Pass");
    open_gl::tech::Shadow::dispatch_shadow_pass(
        m_dir_light_shadow_buffer, m_dir_light_shadow_shader->m_data, dir,
        scenes, m_window_resolution);
  }
  {
    TracyGpuZone("Direct Lighting Pass");
    open_gl::tech::PBRLighting::dispatch_light_pass(
        m_lighting_shader->m_data, m_lightpass_buffer, m_gbuffer,
        m_dir_light_shadow_buffer, cam, point_lights, dir);
  }

  {
    TracyGpuZone("GBuffer Downsample");
    m_gbuffer_downsample.bind();
    open_gl::tech::Utils::dispatch_present_image(
        m_downsample_shader->m_data, "u_prev_mip", 0,
        m_gbuffer.m_colour_attachments[2]);
    m_gbuffer_downsample.unbind();
  }

  {
    TracyGpuZone("Light Pass TAA");

    open_gl::tech::TemporalAntiAliasing::dispatch_taa_pass(
        m_taa_shader->m_data, m_lightpass_buffer, m_lightpass_buffer_resolve,
        m_lightpass_buffer_history, m_gbuffer.m_colour_attachments[4],
        m_window_resolution);
  }

  if (m_debug_draw_cone_tracing_pass || m_debug_draw_cone_tracing_pass_no_taa) {
    TracyGpuZone("Voxel Cone Tracing Pass");
    open_gl::tech::VXGI::dispatch_cone_tracing_pass(
        m_voxel_cone_tracing_shader->m_data, m_voxel_data, m_conetracing_buffer,
        m_gbuffer, m_window_resolution, m_voxel_data.current_bounding_box,
        s_voxel_resolution, cam, m_vxgi_cone_trace_distance,
        m_vxgi_resolution_scale, m_vxgi_diffuse_specular_mix);
  }

  if (m_debug_draw_lighting_pass) {
    open_gl::tech::Utils::dispatch_present_image(
        m_present_shader->m_data, "u_image_sampler", 0,
        m_lightpass_buffer_resolve.m_colour_attachments.front());
  }

  m_ssr_buffer_resolve.bind();
  glClear(GL_COLOR_BUFFER_BIT);
  m_ssr_buffer_resolve.unbind();

  if (m_debug_draw_ssr_pass) {
    TracyGpuZone("SSR Pass");
    glViewport(0, 0, m_window_resolution.x * m_ssr_resolution_scale,
               m_window_resolution.y * m_ssr_resolution_scale);
    open_gl::tech::ScreenSpaceReflections::dispatch_ssr_pass(
        m_ssr_shader->m_data, cam, m_ssr_buffer, m_gbuffer, m_lightpass_buffer,
        m_window_resolution * m_ssr_resolution_scale);
    glViewport(0, 0, m_window_resolution.x, m_window_resolution.y);
    open_gl::tech::TemporalAntiAliasing::dispatch_taa_pass(
        m_taa_shader->m_data, m_ssr_buffer, m_ssr_buffer_resolve,
        m_ssr_buffer_history, m_gbuffer.m_colour_attachments[4],
        m_window_resolution);
  }

  if (m_debug_draw_cone_tracing_pass) {
    {
      TracyGpuZone("Voxel Cone Tracing TAA");
      open_gl::tech::TemporalAntiAliasing::dispatch_taa_pass(
          m_taa_shader->m_data, m_conetracing_buffer,
          m_conetracing_buffer_resolve, m_conetracing_buffer_history,
          m_gbuffer.m_colour_attachments[4], m_window_resolution);

      glViewport(0, 0, m_window_resolution.x * m_vxgi_resolution_scale,
                 m_window_resolution.y * m_vxgi_resolution_scale);
    }
    {
      TracyGpuZone("Voxel Cone Tracing Denoise");
      open_gl::tech::Utils::dispatch_denoise_image(
          m_denoise_shader->m_data, m_conetracing_buffer_resolve,
          m_conetracing_buffer_denoise, m_denoise_sigma, m_denoise_threshold,
          m_denoise_k_sigma, m_window_resolution);
      Texture::bind_sampler_handle(0, GL_TEXTURE0);
      glViewport(0, 0, m_window_resolution.x, m_window_resolution.y);
    }
  }
  if (m_debug_draw_cone_tracing_pass_no_taa) {
    open_gl::tech::Utils::dispatch_present_image(
        m_present_shader->m_data, "u_image_sampler", 0,
        m_conetracing_buffer.m_colour_attachments.front());
  }
  if (m_debug_draw_lighting_pass_no_taa) {
    open_gl::tech::Utils::dispatch_present_image(
        m_present_shader->m_data, "u_image_sampler", 0,
        m_lightpass_buffer.m_colour_attachments.front());
  }

  if (m_debug_draw_ssr_pass) {
    open_gl::tech::Utils::dispatch_present_image(
        m_present_shader->m_data, "u_image_sampler", 0,
        m_ssr_buffer_resolve.m_colour_attachments.front());
  }
  {
    TracyGpuZone("Blit lightpass to history");
    open_gl::tech::Utils::blit_to_fb(
        m_lightpass_buffer_history, m_present_shader->m_data, "u_image_sampler",
        0, m_lightpass_buffer_resolve.m_colour_attachments[0]);
  }
  {
    TracyGpuZone("Blit Gbuffer position to history");
    open_gl::tech::Utils::blit_to_fb(
        m_position_buffer_history, m_present_shader->m_data, "u_image_sampler",
        0, m_gbuffer.m_colour_attachments[1]);
  }
  {
    TracyGpuZone("Blit voxel cone tracing to history");
    open_gl::tech::Utils::blit_to_fb(
        m_conetracing_buffer_history, m_present_shader->m_data,
        "u_image_sampler", 0,
        m_conetracing_buffer_denoise.m_colour_attachments.front());
  }
  {
    TracyGpuZone("Blit ssr pass to history");
    open_gl::tech::Utils::blit_to_fb(
        m_ssr_buffer_history, m_present_shader->m_data, "u_image_sampler", 0,
        m_ssr_buffer_resolve.m_colour_attachments.front());
  }

  glClear(GL_DEPTH_BUFFER_BIT);
  if (m_debug_draw_3d_texture) {
    m_voxel_visualiser.dispatch_draw(m_voxel_data, cam);
  }
  m_voxel_data.previous_bounding_box = m_voxel_data.current_bounding_box;
  glClear(GL_DEPTH_BUFFER_BIT);
  if (m_debug_draw_final_pass) {
    TracyGpuZone("Composite Final Pass");
    GEM_GPU_MARKER("Composite Final Pass");
    m_final_pass.bind();
    Shapes::s_screen_quad.use();
    m_combine_shader->m_data.use();
    m_combine_shader->m_data.set_float("u_brightness",
                                       m_tonemapping_brightness);
    m_combine_shader->m_data.set_float("u_contrast", m_tonemapping_contrast);
    m_combine_shader->m_data.set_float("u_saturation",
                                       m_tonemapping_saturation);
    m_combine_shader->m_data.set_int("lighting_pass", 0);
    Texture::bind_sampler_handle(
        m_lightpass_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE0);
    m_combine_shader->m_data.set_int("cone_tracing_pass", 1);
    Texture::bind_sampler_handle(
        m_conetracing_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE1);
    m_combine_shader->m_data.set_int("ssr_pass", 2);
    m_combine_shader->m_data.set_int("ssr_pass", 2);
    Texture::bind_sampler_handle(
        m_ssr_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    Texture::bind_sampler_handle(0, GL_TEXTURE0);
    Texture::bind_sampler_handle(0, GL_TEXTURE1);
    m_final_pass.unbind();
    open_gl::tech::Utils::dispatch_present_image(
        m_present_shader->m_data, "u_image_sampler", 0,
        m_final_pass.m_colour_attachments.front());

    m_conetracing_buffer_resolve.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    m_conetracing_buffer.unbind();
  }

  {
    TracyGpuZone("Im3D Pass");
    GLIm3d::end_frame_im3d(m_im3d_state, m_window_resolution, cam);
  }
  TracyGpuCollect;
}

void GLRenderer::cleanup(AssetManager &am) {
  ZoneScoped;
  m_gbuffer.cleanup();
  m_gbuffer_downsample.cleanup();
  m_dir_light_shadow_buffer.cleanup();
  m_lightpass_buffer.cleanup();
  m_lightpass_buffer_resolve.cleanup();
  m_lightpass_buffer_history.cleanup();
  m_position_buffer_history.cleanup();
  m_conetracing_buffer.cleanup();
  m_conetracing_buffer_denoise.cleanup();
  m_conetracing_buffer_resolve.cleanup();
  m_conetracing_buffer_history.cleanup();
  m_ssr_buffer.cleanup();
  m_ssr_buffer_denoise.cleanup();
  m_ssr_buffer_resolve.cleanup();
  m_ssr_buffer_history.cleanup();
  m_final_pass.cleanup();
  GLIm3d::shutdown_im3d(m_im3d_state);
}

entt::entity GLRenderer::get_mouse_entity(glm::vec2 mouse_position) {
  ZoneScoped;
  auto pixels = m_gbuffer.read_pixels<glm::vec4, 1, 1>(
      mouse_position.x, m_window_resolution.y - mouse_position.y, 5, GL_RGBA,
      GL_FLOAT);
  m_last_selected_entity = entt::entity(pixels[0][0] + pixels[0][1] * 256 +
                                        pixels[0][2] * 256 * 256);

  return m_last_selected_entity;
}

void GLRenderer::on_imgui(AssetManager &am) {
  ZoneScoped;
  glm::vec2 mouse_pos = Input::get_mouse_position();
  ImGui::Begin("Renderer Settings");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / GPUBackend::selected()->m_imgui_io->Framerate,
              GPUBackend::selected()->m_imgui_io->Framerate);
  ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
  ImGui::Text("Selected Entity ID : %d", m_last_selected_entity);

  ImGui::Checkbox("Simulate Low Framerate", &m_debug_simulate_low_framerate);
  ImGui::Separator();



  if (ImGui::TreeNode("Render Passes")) {
    ImGui::Checkbox("Render 3D Voxel Grid", &m_debug_draw_3d_texture);
    ImGui::Checkbox("Render Final Pass", &m_debug_draw_final_pass);
    ImGui::Checkbox("Render Direct Lighting Pass", &m_debug_draw_lighting_pass);
    ImGui::Checkbox("Render Direct Lighting Pass NO TAA",
                    &m_debug_draw_lighting_pass_no_taa);
    ImGui::Checkbox("Render Cone Tracing Pass",
                    &m_debug_draw_cone_tracing_pass);
    ImGui::Checkbox("Render SSR", &m_debug_draw_ssr_pass);
    ImGui::Checkbox("Render Cone Tracing Pass NO TAA",
                    &m_debug_draw_cone_tracing_pass_no_taa);
    ImGui::Separator();
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Brightness / Contrast / Saturation")) {
    ImGui::DragFloat("Brightness", &m_tonemapping_brightness);
    ImGui::DragFloat("Contrast", &m_tonemapping_contrast);
    ImGui::DragFloat("Saturation", &m_tonemapping_saturation);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("VXGI Settings")) {
    if (ImGui::Button("Clear Voxel Texture")) {
      p_clear_voxel_grid = true;
    }
    ImGui::Checkbox("Freeze Voxel Grid", &m_debug_freeze_voxel_grid_pos);
    ImGui::DragFloat("Trace Distance", &m_vxgi_cone_trace_distance);
    ImGui::DragFloat("Diffuse / Spec Mix", &m_vxgi_diffuse_specular_mix, 1.0f,
                     0.0f, 1.0f);
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("VXGI Voxel Grid Debug")) {
    ImGui::DragFloat3("AABB Dimensions", &m_voxel_data.aabb_dim[0]);
    ImGui::DragFloat("AABB Debug Visual Model Matrix Scale",
                     &m_voxel_visualiser.m_debug_scale, 0.01f, 3000.0f);
    ImGui::DragFloat3("AABB Position Offset",
                      &m_voxel_visualiser.m_debug_position_offset[0]);
    ImGui::DragFloat3("Current VXGI BB Min",
                      &m_voxel_data.current_bounding_box.m_min[0]);
    ImGui::DragFloat3("Current VXGI BB Max",
                      &m_voxel_data.current_bounding_box.m_max[0]);
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Denoise Settings")) {
    ImGui::DragFloat("Sigma", &m_denoise_sigma);
    ImGui::DragFloat("Threshold", &m_denoise_threshold);
    ImGui::DragFloat("KSigma", &m_denoise_k_sigma);
    ImGui::TreePop();
  }
  ImGui::End();

  // TODO: Find a better place for this jesus
  Im3d::DrawAlignedBox(ToIm3D(m_voxel_data.current_bounding_box.m_min),
                       ToIm3D(m_voxel_data.current_bounding_box.m_max));
}
} // namespace gem