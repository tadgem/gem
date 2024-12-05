#pragma once
#include "gem/gl/gl_framebuffer.h"
#include "gem/shape.h"
#include "gem/voxelisation.h"

namespace gem {

class camera;
namespace open_gl {
namespace tech {
class vxgi {
public:
  static void dispatch_voxelisation_gbuffer_pass(gl_shader &gbuffer_shader,
                                                 voxel::grid &grid_data,
                                                 gl_framebuffer gbuffer,
                                                 gl_framebuffer lighting_buffer,
                                                 glm::ivec2 window_res);

  static void dispatch_gbuffer_voxelization(gl_shader &voxelization,
                                            voxel::grid &voxel_data,
                                            gl_framebuffer &gbuffer,
                                            gl_framebuffer &lightpass_buffer,
                                            glm::ivec2 window_res);

  static void dispatch_blit_voxel(gl_shader &blit_voxel,
                                  voxel::grid &voxel_data,
                                  glm::vec3 _3d_tex_res_vec);

  static void dispatch_clear_voxel(gl_shader &clear_voxel,
                                   voxel::grid &voxel_data,
                                   glm::vec3 _3d_tex_res_vec);

  static void dispatch_gen_voxel_mips(gl_shader &voxelization_mips,
                                      voxel::grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec);

  static void dispatch_cone_tracing_pass(
      gl_shader &voxel_cone_tracing, voxel::grid &voxel_data,
      gl_framebuffer &buffer_conetracing, gl_framebuffer &gbuffer,
      glm::ivec2 window_res, aabb &bounding_volume, glm::vec3 _3d_tex_res,
      camera &cam, float max_trace_distance, float resolution_scale,
      float diffuse_spec_mix);
};
} // namespace tech
} // namespace open_gl
} // namespace gem