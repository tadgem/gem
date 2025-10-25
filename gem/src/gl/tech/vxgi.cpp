#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/tech/vxgi.h"
#include "gem/backend.h"
#include "gem/camera.h"
#include "gem/gl/gl_dbg.h"
#include "gem/profile.h"

namespace gem {
namespace gl {

void tech::VXGI::DispatchGBufferVoxelization(GLShader &voxelization,
                                               Voxel::Grid &voxel_data,
                                               GLFramebuffer &gbuffer,
                                               GLFramebuffer &lightpass_buffer,
                                               glm::ivec2 window_res) {
  ZoneScoped;
  GEM_GPU_MARKER("GBuffer Voxelisation");
  voxelization.Use();
  voxelization.SetInt("u_gbuffer_pos", 0);
  voxelization.SetInt("u_gbuffer_lighting", 1);
  voxelization.SetVec3f("u_voxel_resolution", voxel_data.resolution);
  voxelization.SetVec2f("u_input_resolution", {window_res.x, window_res.y});
  voxelization.SetVec3f("u_aabb.min", voxel_data.current_bounding_box.min);
  voxelization.SetVec3f("u_aabb.max", voxel_data.current_bounding_box.max);
  voxelization.SetVec3f("u_voxel_unit", voxel_data.voxel_unit);
  Texture::BindImageHandle(voxel_data.voxel_texture.handle, 0, 0,
                             GL_RGBA16F);
  Texture::BindSamplerHandle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
  Texture::BindSamplerHandle(lightpass_buffer.m_colour_attachments[0],
                               GL_TEXTURE1);
  glAssert(glDispatchCompute(window_res.x / 10, window_res.y / 10, 1));
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void tech::VXGI::DispatchGenerate3DTextureMips(GLShader &voxelization_mips,
                                         Voxel::Grid &voxel_data,
                                         glm::vec3 _3d_tex_res_vec) {
  ZoneScoped;
  GEM_GPU_MARKER("Voxel Mips Generation");
  constexpr int MAX_MIPS = 5;
  voxelization_mips.Use();
  // for each mip in remaining_mipps
  glm::vec3 last_mip_resolution = _3d_tex_res_vec;
  glm::vec3 current_mip_resolution = _3d_tex_res_vec / 2.0f;
  for (int i = 1; i < MAX_MIPS; i++) {
    glBindTexture(GL_TEXTURE_3D, voxel_data.voxel_texture.handle);
    Texture::BindImageHandle(voxel_data.voxel_texture.handle, 0, i,
                               GL_RGBA16F);
    Texture::BindImageHandle(voxel_data.voxel_texture.handle, 1, i - 1,
                               GL_RGBA16F);
    voxelization_mips.SetVec3f("u_current_resolution", current_mip_resolution);
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

void tech::VXGI::DispatchConeTracingPass(
    GLShader &voxel_cone_tracing, Voxel::Grid &voxel_data,
    GLFramebuffer &buffer_conetracing, GLFramebuffer &gbuffer,
    glm::ivec2 window_res, AABB &bounding_volume, glm::vec3 _3d_tex_res,
    Camera &cam, float max_trace_distance, float resolution_scale,
    float diffuse_spec_mix) {
  ZoneScoped;
  GEM_GPU_MARKER("Cone Tracing Pass");

  glBindTexture(GL_TEXTURE_3D, voxel_data.voxel_texture.handle);

  glViewport(0, 0, window_res.x * resolution_scale,
             window_res.y * resolution_scale);
  Shapes::kScreenQuad.Use();
  buffer_conetracing.Bind();
  voxel_cone_tracing.Use();
  voxel_cone_tracing.SetVec3f("u_aabb.min", bounding_volume.min);
  voxel_cone_tracing.SetVec3f("u_aabb.max", bounding_volume.max);
  voxel_cone_tracing.SetVec3f("u_voxel_resolution", _3d_tex_res);
  voxel_cone_tracing.SetInt("u_position_map", 0);
  voxel_cone_tracing.SetVec3f("u_cam_position", cam.position);
  voxel_cone_tracing.SetFloat("u_max_trace_distance", max_trace_distance);
  voxel_cone_tracing.SetFloat("u_diffuse_spec_mix", diffuse_spec_mix);

  Texture::BindSamplerHandle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
  voxel_cone_tracing.SetInt("u_normal_map", 1);
  Texture::BindSamplerHandle(gbuffer.m_colour_attachments[2], GL_TEXTURE1);
  voxel_cone_tracing.SetInt("u_voxel_map", 2);
  Texture::BindSamplerHandle(voxel_data.voxel_texture.handle, GL_TEXTURE2,
                               GL_TEXTURE_3D);
  voxel_cone_tracing.SetInt("u_colour_map", 3);
  Texture::BindSamplerHandle(gbuffer.m_colour_attachments[0], GL_TEXTURE3);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  buffer_conetracing.Unbind();
  Texture::BindSamplerHandle(0, GL_TEXTURE0);
  Texture::BindSamplerHandle(0, GL_TEXTURE1);
  Texture::BindSamplerHandle(0, GL_TEXTURE2);
  Texture::BindSamplerHandle(0, GL_TEXTURE3);
  glViewport(0, 0, window_res.x, window_res.y);
}

void tech::VXGI::DispatchBlit3DTexture(GLShader &blit_voxel,
                                     Voxel::Grid &voxel_data,
                                     glm::vec3 _3d_tex_res_vec) {
  ZoneScoped;
  GEM_GPU_MARKER("Voxel History Blit");
  blit_voxel.Use();
  Texture::BindImageHandle(voxel_data.voxel_texture.handle, 0, 0,
                             GL_RGBA16F);
  blit_voxel.SetVec3f("u_voxel_resolution", _3d_tex_res_vec);

  glAssert(glDispatchCompute(_3d_tex_res_vec.x / 8, _3d_tex_res_vec.y / 8,
                             _3d_tex_res_vec.z / 8));
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
void tech::VXGI::DispatchClear3DTexture(GLShader &clear_voxel,
                                      Voxel::Grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec) {
  ZoneScoped;
  GEM_GPU_MARKER("Clear Voxel Grid");
  clear_voxel.Use();
  Texture::BindImageHandle(voxel_data.voxel_texture.handle, 0, 0,
                             GL_RGBA16F);
  glAssert(glDispatchCompute(_3d_tex_res_vec.x / 8, _3d_tex_res_vec.y / 8,
                             _3d_tex_res_vec.z / 8));
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
} // namespace open_gl
} // namespace gem