#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/tech/vxgi.h"
#include "gem/backend.h"
#include "gem/camera.h"
#include "gem/gl/gl_dbg.h"
#include "gem/profile.h"

namespace gem {
namespace open_gl {

void tech::vxgi::dispatch_gbuffer_voxelization(gl_shader &voxelization,
                                               voxel::grid &voxel_data,
                                               gl_framebuffer &gbuffer,
                                               gl_framebuffer &lightpass_buffer,
                                               glm::ivec2 window_res) {
  ZoneScoped;
  GEM_GPU_MARKER("GBuffer Voxelisation");
  voxelization.use();
  voxelization.set_int("u_gbuffer_pos", 0);
  voxelization.set_int("u_gbuffer_lighting", 1);
  voxelization.set_vec3("u_voxel_resolution", voxel_data.resolution);
  voxelization.set_vec2("u_input_resolution", {window_res.x, window_res.y});
  voxelization.set_vec3("u_aabb.min", voxel_data.current_bounding_box.min);
  voxelization.set_vec3("u_aabb.max", voxel_data.current_bounding_box.max);
  voxelization.set_vec3("u_voxel_unit", voxel_data.voxel_unit);
  texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 0, 0,
                             GL_RGBA16F);
  texture::bind_sampler_handle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
  texture::bind_sampler_handle(lightpass_buffer.m_colour_attachments[0],
                               GL_TEXTURE1);
  glAssert(glDispatchCompute(window_res.x / 10, window_res.y / 10, 1));
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void tech::vxgi::dispatch_gen_voxel_mips(gl_shader &voxelization_mips,
                                         voxel::grid &voxel_data,
                                         glm::vec3 _3d_tex_res_vec) {
  ZoneScoped;
  GEM_GPU_MARKER("Voxel Mips Generation");
  constexpr int MAX_MIPS = 5;
  voxelization_mips.use();
  // for each mip in remaining_mipps
  glm::vec3 last_mip_resolution = _3d_tex_res_vec;
  glm::vec3 current_mip_resolution = _3d_tex_res_vec / 2.0f;
  for (int i = 1; i < MAX_MIPS; i++) {
    glBindTexture(GL_TEXTURE_3D, voxel_data.voxel_texture.m_handle);
    texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 0, i,
                               GL_RGBA16F);
    texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 1, i - 1,
                               GL_RGBA16F);
    voxelization_mips.set_vec3("u_current_resolution", current_mip_resolution);
    glm::ivec3 dispatch_dims =
        glm::ivec3(int(current_mip_resolution.x), int(current_mip_resolution.y),
                   int(current_mip_resolution.z));
    glAssert(glDispatchCompute(dispatch_dims.x / 8, dispatch_dims.y / 8,
                               dispatch_dims.z / 8));
    current_mip_resolution /= 2.0f;
    last_mip_resolution /= 2.0f;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void tech::vxgi::dispatch_cone_tracing_pass(
    gl_shader &voxel_cone_tracing, voxel::grid &voxel_data,
    gl_framebuffer &buffer_conetracing, gl_framebuffer &gbuffer,
    glm::ivec2 window_res, aabb &bounding_volume, glm::vec3 _3d_tex_res,
    camera &cam, float max_trace_distance, float resolution_scale,
    float diffuse_spec_mix) {
  ZoneScoped;
  GEM_GPU_MARKER("Cone Tracing Pass");

  glBindTexture(GL_TEXTURE_3D, voxel_data.voxel_texture.m_handle);

  glViewport(0, 0, window_res.x * resolution_scale,
             window_res.y * resolution_scale);
  shapes::s_screen_quad.use();
  buffer_conetracing.bind();
  voxel_cone_tracing.use();
  voxel_cone_tracing.set_vec3("u_aabb.min", bounding_volume.min);
  voxel_cone_tracing.set_vec3("u_aabb.max", bounding_volume.max);
  voxel_cone_tracing.set_vec3("u_voxel_resolution", _3d_tex_res);
  voxel_cone_tracing.set_int("u_position_map", 0);
  voxel_cone_tracing.set_vec3("u_cam_position", cam.m_pos);
  voxel_cone_tracing.set_float("u_max_trace_distance", max_trace_distance);
  voxel_cone_tracing.set_float("u_diffuse_spec_mix", diffuse_spec_mix);

  texture::bind_sampler_handle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
  voxel_cone_tracing.set_int("u_normal_map", 1);
  texture::bind_sampler_handle(gbuffer.m_colour_attachments[2], GL_TEXTURE1);
  voxel_cone_tracing.set_int("u_voxel_map", 2);
  texture::bind_sampler_handle(voxel_data.voxel_texture.m_handle, GL_TEXTURE2,
                               GL_TEXTURE_3D);
  voxel_cone_tracing.set_int("u_colour_map", 3);
  texture::bind_sampler_handle(gbuffer.m_colour_attachments[0], GL_TEXTURE3);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  buffer_conetracing.unbind();
  texture::bind_sampler_handle(0, GL_TEXTURE0);
  texture::bind_sampler_handle(0, GL_TEXTURE1);
  texture::bind_sampler_handle(0, GL_TEXTURE2);
  texture::bind_sampler_handle(0, GL_TEXTURE3);
  glViewport(0, 0, window_res.x, window_res.y);
}

void tech::vxgi::dispatch_blit_voxel(gl_shader &blit_voxel,
                                     voxel::grid &voxel_data,
                                     glm::vec3 _3d_tex_res_vec) {
  ZoneScoped;
  GEM_GPU_MARKER("Voxel History Blit");
  blit_voxel.use();
  texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 0, 0,
                             GL_RGBA16F);
  blit_voxel.set_vec3("u_voxel_resolution", _3d_tex_res_vec);

  glAssert(glDispatchCompute(_3d_tex_res_vec.x / 8, _3d_tex_res_vec.y / 8,
                             _3d_tex_res_vec.z / 8));
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
void tech::vxgi::dispatch_clear_voxel(gl_shader &clear_voxel,
                                      voxel::grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec) {
  ZoneScoped;
  GEM_GPU_MARKER("Clear Voxel Grid");
  clear_voxel.use();
  texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 0, 0,
                             GL_RGBA16F);
  glAssert(glDispatchCompute(_3d_tex_res_vec.x / 8, _3d_tex_res_vec.y / 8,
                             _3d_tex_res_vec.z / 8));
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
void tech::vxgi::dispatch_voxelisation_gbuffer_pass(
    gl_shader &gbuffer_shader, voxel::grid &grid_data, gl_framebuffer gbuffer,
    gl_framebuffer lighting_buffer, glm::ivec2 window_res) {}
} // namespace open_gl
} // namespace gem