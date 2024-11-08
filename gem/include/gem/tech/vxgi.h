#pragma once
#include "gem/framebuffer.h"
#include "gem/shape.h"
#include "gem/voxelisation.h"

namespace gem {

class camera;

namespace tech {
class vxgi {
public:
  static void dispatch_gbuffer_voxelization(shader &voxelization,
                                            aabb &volume_bounding_box,
                                            voxel::grid &voxel_data,
                                            framebuffer &gbuffer,
                                            framebuffer &lightpass_buffer,
                                            glm::ivec2 window_res);

  static void dispatch_blit_voxel(shader &blit_voxel,
                                      voxel::grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec);


  static void dispatch_gen_voxel_mips(shader &voxelization_mips,
                                      voxel::grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec);

  static void dispatch_voxel_reprojection(shader &voxel_reprojection,
                                      voxel::grid &voxel_data,
                                      glm::vec3 _3d_tex_res_vec, aabb old_bb, aabb new_bb);

  static void dispatch_cone_tracing_pass(
      shader &voxel_cone_tracing, voxel::grid &voxel_data,
      framebuffer &buffer_conetracing, framebuffer &gbuffer,
      glm::ivec2 window_res, aabb &bounding_volume, glm::vec3 _3d_tex_res,
      camera &cam, float max_trace_distance, float resolution_scale,
      float diffuse_spec_mix);
};
} // namespace tech
} // namespace gem