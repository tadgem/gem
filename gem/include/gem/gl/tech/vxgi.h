#pragma once
#include "gem/gl/gl_framebuffer.h"
#include "gem/shape.h"
#include "gem/voxelisation.h"

namespace gem {

class Camera;
namespace gl {
namespace tech {
class VXGI {
public:
  static void DispatchGBufferVoxelization(GLShader &voxelization,
                                            Voxel::Grid &voxel_data,
                                            GLFramebuffer &gbuffer,
                                            GLFramebuffer &lightpass_buffer,
                                            glm::ivec2 window_res);

  static void DispatchBlit3DTexture(GLShader &blit_voxel,
                                  Voxel::Grid &voxel_data,
                                  glm::vec3 _3d_tex_res_vec);

  static void DispatchClear3DTexture(GLShader &clear_voxel,
                                   Voxel::Grid &voxel_data,
                                   glm::vec3 _3d_tex_res_vec);

  static void DispatchGenerate3DTextureMips(GLShader &voxelization_mips,
                                      Voxel::Grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec);

  static void DispatchConeTracingPass(
      GLShader &voxel_cone_tracing, Voxel::Grid &voxel_data,
      GLFramebuffer &buffer_conetracing, GLFramebuffer &gbuffer,
      glm::ivec2 window_res, AABB &bounding_volume, glm::vec3 _3d_tex_res,
      Camera &cam, float max_trace_distance, float resolution_scale,
      float diffuse_spec_mix);
};
} // namespace tech
} // namespace gl
} // namespace gem