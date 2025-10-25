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

void GLRenderer::Init(AssetManager &am, glm::ivec2 resolution) {
  ZoneScoped;
  TracyGpuContext;
  frame_index = 0;
  im3d_state = GLIm3d::LoadIm3D();

  am.LoadAsset("assets/shaders/gbuffer.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/gbuffer_textureless.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/lighting.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/forward_lighting.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/visualize_3d_tex.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/visualize_3d_tex_instances.shader",
                AssetType::kShader);
  am.LoadAsset("assets/shaders/present.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/dir_light_shadow.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/voxel_cone_tracing.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/ssr.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/taa.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/denoise.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/gi_combine.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/downsample.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/gbuffer_voxelization.shader",
                AssetType::kShader);
  am.LoadAsset("assets/shaders/voxel_mips.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/voxel_reprojection.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/voxel_blit.shader", AssetType::kShader);
  am.LoadAsset("assets/shaders/voxel_clear.shader", AssetType::kShader);

  am.WaitAllLoads();
  gbuffer_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/gbuffer.shader");
  gbuffer_textureless_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/gbuffer_textureless.shader");
  forward_lighting_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/forward_lighting.shader");
  lighting_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/lighting.shader");
  visualise_3d_tex_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/visualize_3d_tex.shader");
  visualise_3d_tex_instances_shader =
      am.GetAsset<GLShader, AssetType::kShader>(
          "assets/shaders/visualize_3d_tex_instances.shader");
  present_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/present.shader");
  dir_light_shadow_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/dir_light_shadow.shader");
  voxel_cone_tracing_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/voxel_cone_tracing.shader");
  ssr_shader =
      am.GetAsset<GLShader, AssetType::kShader>("assets/shaders/ssr.shader");
  taa_shader =
      am.GetAsset<GLShader, AssetType::kShader>("assets/shaders/taa.shader");
  denoise_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/denoise.shader");
  combine_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/gi_combine.shader");
  downsample_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/downsample.shader");
  compute_voxelize_gbuffer_shader =
      am.GetAsset<GLShader, AssetType::kShader>(
          "assets/shaders/gbuffer_voxelization.shader");
  compute_voxel_mips_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/voxel_mips.shader");
  compute_voxel_reprojection_shader =
      am.GetAsset<GLShader, AssetType::kShader>(
          "assets/shaders/voxel_reprojection.shader");
  compute_voxel_blit_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/voxel_blit.shader");
  compute_voxel_clear_shader = am.GetAsset<GLShader, AssetType::kShader>(
      "assets/shaders/voxel_clear.shader");

  window_resolution = resolution;
  const int shadow_resolution = 4096;
  gbuffer =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA32F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                                 {GL_RGB, GL_RGB16F, GL_NEAREST, GL_FLOAT},
                             },
                             true);

  gbuffer_downsample =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  dir_light_shadow_buffer =
      GLFramebuffer::Create({shadow_resolution, shadow_resolution}, {}, true);

  lightpass_buffer =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  lightpass_buffer_resolve =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  lightpass_buffer_history =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  position_buffer_history =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  glm::vec2 gi_res = {window_resolution.x * vxgi_resolution_scale,
                      window_resolution.y * vxgi_resolution_scale};
  conetracing_buffer =
      GLFramebuffer::Create(gi_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  conetracing_buffer_denoise =
      GLFramebuffer::Create(gi_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  conetracing_buffer_resolve =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  conetracing_buffer_history =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  glm::vec2 ssr_res = {window_resolution.x * ssr_resolution_scale,
                       window_resolution.y * ssr_resolution_scale};
  ssr_buffer =
      GLFramebuffer::Create(ssr_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  ssr_buffer_denoise =
      GLFramebuffer::Create(ssr_res,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  ssr_buffer_resolve =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  ssr_buffer_history =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  final_pass =
      GLFramebuffer::Create(window_resolution,
                             {
                                 {GL_RGBA, GL_RGBA8, GL_LINEAR, GL_FLOAT},
                             },
                             false);

  voxel_data = Voxel::CreateGrid(kVoxelResolution, AABB{});
  Camera cam{}; // TODO: clean this up, just need a position of 0,0,0 to init
  voxel_data.UpdateVoxelUnit();
  voxel_visualiser = Voxel::CreateGridVisualizer(
      voxel_data, visualise_3d_tex_shader->data,
      visualise_3d_tex_instances_shader->data, 8);
}

void GLRenderer::PreFrame(Camera &cam) {
  ZoneScoped;

  GLIm3d::NewFrameIm3D(im3d_state, window_resolution, cam);
}

void GLRenderer::Render(AssetManager &am, Camera &cam,
                         std::vector<Scene *> &scenes) {
  ZoneScoped;
  FrameMark;

  glEnable(GL_DEPTH_TEST);

  if(debug_simulate_low_framerate)
  {
    SDL_Delay(50);
  }

  if (clear_voxel_grid_) {
    gl::tech::VXGI::DispatchClear3DTexture(
        compute_voxel_clear_shader->data, voxel_data, kVoxelResolution);
    clear_voxel_grid_ = false;
  }

  voxel_data.UpdateVoxelUnit();

  {
    TracyGpuZone("GBuffer Voxelization");
    gl::tech::VXGI::DispatchGBufferVoxelization(
        compute_voxelize_gbuffer_shader->data, voxel_data, gbuffer,
        lightpass_buffer, window_resolution);
  }

  //{
  //  TracyGpuZone("GBuffer Voxelization MIPS");
  //  open_gl::tech::VXGI::dispatch_gen_voxel_mips(
  //      compute_voxel_mips_shader->data, voxel_data, s_voxel_resolution);
  //}

  {
    TracyGpuZone("GBuffer");
    gbuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl::tech::GBuffer::DispatchGBufferWithID(
        frame_index, gbuffer, position_buffer_history,
        gbuffer_shader->data, am, cam, scenes, window_resolution);

    gl::tech::GBuffer::DispatchGBufferTexturelessWithID(
        frame_index, gbuffer, position_buffer_history,
        gbuffer_textureless_shader->data, am, cam, scenes,
        window_resolution);

    frame_index++;
  }
  // TODO: Need a way to get a single instance more efficiently
  DirectionalLight dir{};
  std::vector<PointLight> point_lights{};
  if (!scenes.empty()) {
    auto dir_light_view = scenes.front()->registry.view<DirectionalLight>();
    for (auto [e, dir_light_c] : dir_light_view.each()) {
      dir = dir_light_c;
      break;
    }
  }
  {
    TracyGpuZone("Dir Light Shadow Pass");
    gl::tech::Shadow::DispatchShadowPass(
        dir_light_shadow_buffer, dir_light_shadow_shader->data, dir,
        scenes, window_resolution);
  }
  {
    TracyGpuZone("Direct Lighting Pass");
    gl::tech::PBRLighting::DispatchLightPass(
        lighting_shader->data, lightpass_buffer, gbuffer,
        dir_light_shadow_buffer, cam, point_lights, dir);
  }

  {
    TracyGpuZone("GBuffer Downsample");
    gbuffer_downsample.Bind();
    gl::tech::Utils::DispatchPresentImage(
        downsample_shader->data, "u_prev_mip", 0,
        gbuffer.colour_attachments[2]);
    gbuffer_downsample.Unbind();
  }

  {
    TracyGpuZone("Light Pass TAA");

    gl::tech::TemporalAntiAliasing::DispatchTAAPass(
        taa_shader->data, lightpass_buffer, lightpass_buffer_resolve,
        lightpass_buffer_history, gbuffer.colour_attachments[4],
        window_resolution);
  }

  if (debug_draw_cone_tracing_pass || debug_draw_cone_tracing_pass_no_taa) {
    TracyGpuZone("Voxel Cone Tracing Pass");
    gl::tech::VXGI::DispatchConeTracingPass(
        voxel_cone_tracing_shader->data, voxel_data, conetracing_buffer,
        gbuffer, window_resolution, voxel_data.current_bounding_box,
        kVoxelResolution, cam, vxgi_cone_trace_distance,
        vxgi_resolution_scale, vxgi_diffuse_specular_mix);
  }

  if (debug_draw_lighting_pass) {
    gl::tech::Utils::DispatchPresentImage(
        present_shader->data, "u_image_sampler", 0,
        lightpass_buffer_resolve.colour_attachments.front());
  }

  ssr_buffer_resolve.Bind();
  glClear(GL_COLOR_BUFFER_BIT);
  ssr_buffer_resolve.Unbind();

  if (debug_draw_ssr_pass) {
    TracyGpuZone("SSR Pass");
    glViewport(0, 0, window_resolution.x * ssr_resolution_scale,
               window_resolution.y * ssr_resolution_scale);
    gl::tech::ScreenSpaceReflections::DispatchSSRPass(
        ssr_shader->data, cam, ssr_buffer, gbuffer, lightpass_buffer,
        window_resolution * ssr_resolution_scale);
    glViewport(0, 0, window_resolution.x, window_resolution.y);
    gl::tech::TemporalAntiAliasing::DispatchTAAPass(
        taa_shader->data, ssr_buffer, ssr_buffer_resolve,
        ssr_buffer_history, gbuffer.colour_attachments[4],
        window_resolution);
  }

  if (debug_draw_cone_tracing_pass) {
    {
      TracyGpuZone("Voxel Cone Tracing TAA");
      gl::tech::TemporalAntiAliasing::DispatchTAAPass(
          taa_shader->data, conetracing_buffer,
          conetracing_buffer_resolve, conetracing_buffer_history,
          gbuffer.colour_attachments[4], window_resolution);

      glViewport(0, 0, window_resolution.x * vxgi_resolution_scale,
                 window_resolution.y * vxgi_resolution_scale);
    }
    {
      TracyGpuZone("Voxel Cone Tracing Denoise");
      gl::tech::Utils::DispatchDenoiseImage(
          denoise_shader->data, conetracing_buffer_resolve,
          conetracing_buffer_denoise, denoise_sigma, denoise_threshold,
          denoise_k_sigma, window_resolution);
      Texture::BindSamplerHandle(0, GL_TEXTURE0);
      glViewport(0, 0, window_resolution.x, window_resolution.y);
    }
  }
  if (debug_draw_cone_tracing_pass_no_taa) {
    gl::tech::Utils::DispatchPresentImage(
        present_shader->data, "u_image_sampler", 0,
        conetracing_buffer.colour_attachments.front());
  }
  if (debug_draw_lighting_pass_no_taa) {
    gl::tech::Utils::DispatchPresentImage(
        present_shader->data, "u_image_sampler", 0,
        lightpass_buffer.colour_attachments.front());
  }

  if (debug_draw_ssr_pass) {
    gl::tech::Utils::DispatchPresentImage(
        present_shader->data, "u_image_sampler", 0,
        ssr_buffer_resolve.colour_attachments.front());
  }
  {
    TracyGpuZone("Blit lightpass to history");
    gl::tech::Utils::DispatchBlitToFB(
        lightpass_buffer_history, present_shader->data, "u_image_sampler",
        0, lightpass_buffer_resolve.colour_attachments[0]);
  }
  {
    TracyGpuZone("Blit Gbuffer position to history");
    gl::tech::Utils::DispatchBlitToFB(
        position_buffer_history, present_shader->data, "u_image_sampler",
        0, gbuffer.colour_attachments[1]);
  }
  {
    TracyGpuZone("Blit voxel cone tracing to history");
    gl::tech::Utils::DispatchBlitToFB(
        conetracing_buffer_history, present_shader->data,
        "u_image_sampler", 0,
        conetracing_buffer_denoise.colour_attachments.front());
  }
  {
    TracyGpuZone("Blit ssr pass to history");
    gl::tech::Utils::DispatchBlitToFB(
        ssr_buffer_history, present_shader->data, "u_image_sampler", 0,
        ssr_buffer_resolve.colour_attachments.front());
  }

  glClear(GL_DEPTH_BUFFER_BIT);
  if (debug_draw_3d_texture) {
    voxel_visualiser.Draw(voxel_data, cam);
  }
  voxel_data.previous_bounding_box = voxel_data.current_bounding_box;
  glClear(GL_DEPTH_BUFFER_BIT);
  if (debug_draw_final_pass) {
    TracyGpuZone("Composite Final Pass");
    GEM_GPU_MARKER("Composite Final Pass");
    final_pass.Bind();
    Shapes::kScreenQuad.Use();
    combine_shader->data.Use();
    combine_shader->data.SetFloat("u_brightness",
                                       tonemapping_brightness);
    combine_shader->data.SetFloat("u_contrast", tonemapping_contrast);
    combine_shader->data.SetFloat("u_saturation",
                                       tonemapping_saturation);
    combine_shader->data.SetInt("lighting_pass", 0);
    Texture::BindSamplerHandle(
        lightpass_buffer_resolve.colour_attachments.front(), GL_TEXTURE0);
    combine_shader->data.SetInt("cone_tracing_pass", 1);
    Texture::BindSamplerHandle(
        conetracing_buffer_resolve.colour_attachments.front(), GL_TEXTURE1);
    combine_shader->data.SetInt("ssr_pass", 2);
    combine_shader->data.SetInt("ssr_pass", 2);
    Texture::BindSamplerHandle(
        ssr_buffer_resolve.colour_attachments.front(), GL_TEXTURE2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    Texture::BindSamplerHandle(0, GL_TEXTURE0);
    Texture::BindSamplerHandle(0, GL_TEXTURE1);
    final_pass.Unbind();
    gl::tech::Utils::DispatchPresentImage(
        present_shader->data, "u_image_sampler", 0,
        final_pass.colour_attachments.front());

    conetracing_buffer_resolve.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    conetracing_buffer.Unbind();
  }

  {
    TracyGpuZone("Im3D Pass");
    GLIm3d::EndFrameIm3D(im3d_state, window_resolution, cam);
  }
  TracyGpuCollect;
}

void GLRenderer::Cleanup(AssetManager &am) {
  ZoneScoped;
  gbuffer.Cleanup();
  gbuffer_downsample.Cleanup();
  dir_light_shadow_buffer.Cleanup();
  lightpass_buffer.Cleanup();
  lightpass_buffer_resolve.Cleanup();
  lightpass_buffer_history.Cleanup();
  position_buffer_history.Cleanup();
  conetracing_buffer.Cleanup();
  conetracing_buffer_denoise.Cleanup();
  conetracing_buffer_resolve.Cleanup();
  conetracing_buffer_history.Cleanup();
  ssr_buffer.Cleanup();
  ssr_buffer_denoise.Cleanup();
  ssr_buffer_resolve.Cleanup();
  ssr_buffer_history.Cleanup();
  final_pass.Cleanup();
  GLIm3d::ShutdownIm3D(im3d_state);
}

entt::entity GLRenderer::GetEntityAtScreenPosition(glm::vec2 mouse_position) {
  ZoneScoped;
  auto pixels = gbuffer.ReadPixels<glm::vec4, 1, 1>(
      mouse_position.x, window_resolution.y - mouse_position.y, 5, GL_RGBA,
      GL_FLOAT);
  last_selected_entity = entt::entity(pixels[0][0] + pixels[0][1] * 256 +
                                        pixels[0][2] * 256 * 256);

  return last_selected_entity;
}

void GLRenderer::OnImGui(AssetManager &am) {
  ZoneScoped;
  glm::vec2 mouse_pos = Input::GetMousePosition();
  ImGui::Begin("Renderer Settings");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / GPUBackend::Selected()->imgui_io->Framerate,
              GPUBackend::Selected()->imgui_io->Framerate);
  ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
  ImGui::Text("Selected Entity ID : %d", last_selected_entity);

  ImGui::Checkbox("Simulate Low Framerate", &debug_simulate_low_framerate);
  ImGui::Separator();



  if (ImGui::TreeNode("Render Passes")) {
    ImGui::Checkbox("Render 3D Voxel Grid", &debug_draw_3d_texture);
    ImGui::Checkbox("Render Final Pass", &debug_draw_final_pass);
    ImGui::Checkbox("Render Direct Lighting Pass", &debug_draw_lighting_pass);
    ImGui::Checkbox("Render Direct Lighting Pass NO TAA",
                    &debug_draw_lighting_pass_no_taa);
    ImGui::Checkbox("Render Cone Tracing Pass",
                    &debug_draw_cone_tracing_pass);
    ImGui::Checkbox("Render SSR", &debug_draw_ssr_pass);
    ImGui::Checkbox("Render Cone Tracing Pass NO TAA",
                    &debug_draw_cone_tracing_pass_no_taa);
    ImGui::Separator();
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Brightness / Contrast / Saturation")) {
    ImGui::DragFloat("Brightness", &tonemapping_brightness);
    ImGui::DragFloat("Contrast", &tonemapping_contrast);
    ImGui::DragFloat("Saturation", &tonemapping_saturation);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("VXGI Settings")) {
    if (ImGui::Button("Clear Voxel Texture")) {
      clear_voxel_grid_ = true;
    }
    ImGui::Checkbox("Freeze Voxel Grid", &debug_freeze_voxel_grid_pos);
    ImGui::DragFloat("Trace Distance", &vxgi_cone_trace_distance);
    ImGui::DragFloat("Diffuse / Spec Mix", &vxgi_diffuse_specular_mix, 1.0f,
                     0.0f, 1.0f);
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("VXGI Voxel Grid Debug")) {
    ImGui::DragFloat3("AABB Dimensions", &voxel_data.aabb_dim[0]);
    ImGui::DragFloat("AABB Debug Visual Model Matrix Scale",
                     &voxel_visualiser.debug_scale, 0.01f, 3000.0f);
    ImGui::DragFloat3("AABB Position Offset",
                      &voxel_visualiser.debug_position_offset[0]);
    ImGui::DragFloat3("Current VXGI BB Min",
                      &voxel_data.current_bounding_box.min[0]);
    ImGui::DragFloat3("Current VXGI BB Max",
                      &voxel_data.current_bounding_box.max[0]);
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Denoise Settings")) {
    ImGui::DragFloat("Sigma", &denoise_sigma);
    ImGui::DragFloat("Threshold", &denoise_threshold);
    ImGui::DragFloat("KSigma", &denoise_k_sigma);
    ImGui::TreePop();
  }
  ImGui::End();

  // TODO: Find a better place for this jesus
  Im3d::DrawAlignedBox(ToIm3D(voxel_data.current_bounding_box.min),
                       ToIm3D(voxel_data.current_bounding_box.max));
}
} // namespace gem