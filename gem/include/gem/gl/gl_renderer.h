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

  void init(AssetManager &am, glm::ivec2 resolution);
  void pre_frame(Camera &cam);
  void render(AssetManager &am, Camera &cam, std::vector<Scene *> &scenes);
  void cleanup(AssetManager &am);

  entt::entity get_mouse_entity(glm::vec2 mouse_position);

  GLShaderAsset *m_gbuffer_shader;
  GLShaderAsset *m_gbuffer_textureless_shader;
  GLShaderAsset *m_forward_lighting_shader;
  GLShaderAsset *m_lighting_shader;
  GLShaderAsset *m_visualise_3d_tex_shader;
  GLShaderAsset *m_visualise_3d_tex_instances_shader;
  GLShaderAsset *m_present_shader;
  GLShaderAsset *m_dir_light_shadow_shader;
  GLShaderAsset *m_voxel_cone_tracing_shader;
  GLShaderAsset *m_ssr_shader;
  GLShaderAsset *m_taa_shader;
  GLShaderAsset *m_denoise_shader;
  GLShaderAsset *m_combine_shader;
  GLShaderAsset *m_downsample_shader;
  GLShaderAsset *m_compute_voxelize_gbuffer_shader;
  GLShaderAsset *m_compute_voxel_mips_shader;
  GLShaderAsset *m_compute_voxel_reprojection_shader;
  GLShaderAsset *m_compute_voxel_blit_shader;
  GLShaderAsset *m_compute_voxel_clear_shader;

  GLFramebuffer m_gbuffer;
  GLFramebuffer m_gbuffer_downsample;
  GLFramebuffer m_dir_light_shadow_buffer;
  GLFramebuffer m_lightpass_buffer;
  GLFramebuffer m_lightpass_buffer_resolve;
  GLFramebuffer m_lightpass_buffer_history;
  GLFramebuffer m_position_buffer_history;
  GLFramebuffer m_conetracing_buffer;
  GLFramebuffer m_conetracing_buffer_denoise;
  GLFramebuffer m_conetracing_buffer_resolve;
  GLFramebuffer m_conetracing_buffer_history;
  GLFramebuffer m_ssr_buffer;
  GLFramebuffer m_ssr_buffer_denoise;
  GLFramebuffer m_ssr_buffer_resolve;
  GLFramebuffer m_ssr_buffer_history;
  GLFramebuffer m_final_pass;

  Im3dState m_im3d_state;
  Voxel::Grid m_voxel_data;
  Voxel::GridVisualizer m_voxel_visualiser;

  glm::vec2     m_window_resolution;
  u32           m_frame_index;
  entt::entity  m_last_selected_entity;

  GLfloat m_vxgi_cone_trace_distance = 45.0f;
  GLfloat m_vxgi_diffuse_specular_mix = 0.0f;
  GLfloat m_vxgi_resolution_scale = 0.75f;

  GLfloat m_ssr_resolution_scale = 1.0f;

  GLfloat m_denoise_sigma = 0.01f;
  GLfloat m_denoise_threshold = 0.1f;
  GLfloat m_denoise_k_sigma = 0.01f;

  GLfloat m_tonemapping_brightness = 0.05f;
  GLfloat m_tonemapping_contrast = 1.07f;
  GLfloat m_tonemapping_saturation = 1.05f;

  bool m_debug_draw_cone_tracing_pass = false;
  bool m_debug_draw_cone_tracing_pass_no_taa = false;
  bool m_debug_draw_lighting_pass = true;
  bool m_debug_draw_lighting_pass_no_taa = false;
  bool m_debug_draw_ssr_pass = true;
  bool m_debug_draw_3d_texture = false;
  bool m_debug_draw_final_pass = true;
  bool m_debug_freeze_voxel_grid_pos = false;
  bool m_debug_simulate_low_framerate = false;

  inline static constexpr int s_voxel_square_resolution = 256;
  inline static constexpr glm::ivec3 s_voxel_resolution =
      glm::ivec3(s_voxel_square_resolution);

  void on_imgui(AssetManager &am);

protected:
  bool p_clear_voxel_grid = false;
};
} // namespace gem