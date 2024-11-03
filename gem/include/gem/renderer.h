#pragma once
#include "gem/asset_definitions.h"
#include "gem/camera.h"
#include "gem/framebuffer.h"
#include "gem/im3d_gl.h"
#include "gem/scene.h"
#include "gem/voxelisation.h"
#include <memory>

namespace gem {

class asset_manager;

class gl_renderer {
public:
  gl_renderer() = default;

  void init(asset_manager &am);
  void pre_frame(camera &cam);
  void render(asset_manager &am, camera &cam, std::vector<scene *> &scenes);
  void cleanup(asset_manager &am);

  entt::entity get_mouse_entity(glm::vec2 mouse_position);

  shader_asset *m_gbuffer_shader;
  shader_asset *m_lighting_shader;
  shader_asset *m_visualise_3d_tex_shader;
  shader_asset *m_present_shader;
  shader_asset *m_dir_light_shadow_shader;
  shader_asset *m_voxel_cone_tracing_shader;
  shader_asset *m_ssr_shader;
  shader_asset *m_taa_shader;
  shader_asset *m_denoise_shader;
  shader_asset *m_combine_shader;
  shader_asset *m_downsample_shader;
  shader_asset *m_compute_voxelize_gbuffer_shader;
  shader_asset *m_compute_voxel_mips_shader;
  shader_asset *m_compute_voxel_reprojection_shader;

  framebuffer m_gbuffer;
  framebuffer m_gbuffer_downsample;
  framebuffer m_dir_light_shadow_buffer;
  framebuffer m_lightpass_buffer;
  framebuffer m_lightpass_buffer_resolve;
  framebuffer m_lightpass_buffer_history;
  framebuffer m_position_buffer_history;
  framebuffer m_conetracing_buffer;
  framebuffer m_conetracing_buffer_denoise;
  framebuffer m_conetracing_buffer_resolve;
  framebuffer m_conetracing_buffer_history;
  framebuffer m_ssr_buffer;
  framebuffer m_ssr_buffer_denoise;
  framebuffer m_ssr_buffer_resolve;
  framebuffer m_ssr_buffer_history;
  framebuffer m_final_pass;

  im3d_state m_im3d_state;
  voxel::grid m_voxel_data;
  voxel::grid_visualiser m_voxel_visualiser;

  glm::vec2 m_window_resolution;
  u32 m_frame_index;
  entt::entity m_last_selected_entity;

  GLfloat m_vxgi_cone_trace_distance = 45.0f;
  GLfloat m_vxgi_diffuse_specular_mix = 0.0f;
  GLfloat m_vxgi_resolution_scale = 0.75f;

  GLfloat m_ssr_resolution_scale = 0.66f;

  GLfloat m_denoise_sigma = 0.01f;
  GLfloat m_denoise_threshold = 0.1f;
  GLfloat m_denoise_k_sigma = 0.01f;

  GLfloat m_tonemapping_brightness = 0.05f;
  GLfloat m_tonemapping_contrast = 1.07f;
  GLfloat m_tonemapping_saturation = 1.05f;

  bool m_debug_draw_cone_tracing_pass = true;
  bool m_debug_draw_cone_tracing_pass_no_taa = false;
  bool m_debug_draw_lighting_pass = true;
  bool m_debug_draw_lighting_pass_no_taa = false;
  bool m_debug_draw_ssr_pass = true;
  bool m_debug_draw_3d_texture = false;
  bool m_debug_draw_final_pass = true;

  inline static constexpr int s_voxel_square_resolution = 256;
  inline static constexpr glm::ivec3 s_voxel_resolution =
      glm::ivec3(s_voxel_square_resolution);

  void on_imgui(asset_manager &am);
};
} // namespace gem