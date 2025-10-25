#pragma once
#include "gem/asset_definitions.h"
#include "gem/camera.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/gl_im3d.h"
#include "gem/scene.h"
#include "gem/voxelisation.h"
#include <memory>

namespace gem {

class AssetManager;

class GLRenderer {
public:
  GLRenderer() = default;

  void Init(AssetManager &am, glm::ivec2 resolution);
  void PreFrame(Camera &cam);
  void Render(AssetManager &am, Camera &cam, std::vector<Scene *> &scenes);
  void Cleanup(AssetManager &am);

  entt::entity GetEntityAtScreenPosition(glm::vec2 mouse_position);

  GLShaderAsset *gbuffer_shader;
  GLShaderAsset *gbuffer_textureless_shader;
  GLShaderAsset *forward_lighting_shader;
  GLShaderAsset *lighting_shader;
  GLShaderAsset *visualise_3d_tex_shader;
  GLShaderAsset *visualise_3d_tex_instances_shader;
  GLShaderAsset *present_shader;
  GLShaderAsset *dir_light_shadow_shader;
  GLShaderAsset *voxel_cone_tracing_shader;
  GLShaderAsset *ssr_shader;
  GLShaderAsset *taa_shader;
  GLShaderAsset *denoise_shader;
  GLShaderAsset *combine_shader;
  GLShaderAsset *downsample_shader;
  GLShaderAsset *compute_voxelize_gbuffer_shader;
  GLShaderAsset *compute_voxel_mips_shader;
  GLShaderAsset *compute_voxel_reprojection_shader;
  GLShaderAsset *compute_voxel_blit_shader;
  GLShaderAsset *compute_voxel_clear_shader;

  GLFramebuffer gbuffer;
  GLFramebuffer gbuffer_downsample;
  GLFramebuffer dir_light_shadow_buffer;
  GLFramebuffer lightpass_buffer;
  GLFramebuffer lightpass_buffer_resolve;
  GLFramebuffer lightpass_buffer_history;
  GLFramebuffer position_buffer_history;
  GLFramebuffer conetracing_buffer;
  GLFramebuffer conetracing_buffer_denoise;
  GLFramebuffer conetracing_buffer_resolve;
  GLFramebuffer conetracing_buffer_history;
  GLFramebuffer ssr_buffer;
  GLFramebuffer ssr_buffer_denoise;
  GLFramebuffer ssr_buffer_resolve;
  GLFramebuffer ssr_buffer_history;
  GLFramebuffer final_pass;

  Im3dState im3d_state;
  Voxel::Grid voxel_data;
  Voxel::GridVisualizer voxel_visualiser;

  glm::vec2     window_resolution;
  u32           frame_index;
  entt::entity  last_selected_entity;

  GLfloat vxgi_cone_trace_distance = 45.0f;
  GLfloat vxgi_diffuse_specular_mix = 0.0f;
  GLfloat vxgi_resolution_scale = 0.75f;

  GLfloat ssr_resolution_scale = 1.0f;

  GLfloat denoise_sigma = 0.01f;
  GLfloat denoise_threshold = 0.1f;
  GLfloat denoise_k_sigma = 0.01f;

  GLfloat tonemapping_brightness = 0.05f;
  GLfloat tonemapping_contrast = 1.07f;
  GLfloat tonemapping_saturation = 1.05f;

  bool debug_draw_cone_tracing_pass = false;
  bool debug_draw_cone_tracing_pass_no_taa = false;
  bool debug_draw_lighting_pass = true;
  bool debug_draw_lighting_pass_no_taa = false;
  bool debug_draw_ssr_pass = true;
  bool debug_draw_3d_texture = false;
  bool debug_draw_final_pass = true;
  bool debug_freeze_voxel_grid_pos = false;
  bool debug_simulate_low_framerate = false;

  inline static constexpr int kVoxelCubedResolution = 256;
  inline static constexpr glm::ivec3 kVoxelResolution =
      glm::ivec3(kVoxelCubedResolution);

  void OnImGui(AssetManager &am);

protected:
  bool clear_voxel_grid_ = false;
};
} // namespace gem