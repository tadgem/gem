#pragma once
#include "gem/aabb.h"
#include "gem/camera.h"
#include "gem/gl/gl_shader.h"
#include "gem/texture.h"

namespace gem {

class voxel {
public:
  struct grid {
    texture voxel_texture; // 3D Texture (Voxel Data)
    glm::ivec3 resolution;
    glm::vec3 voxel_unit; // scale of each texel
    glm::vec3 aabb_dim{200.0, 100.0, 200.0};
    aabb current_bounding_box;
    aabb previous_bounding_box;

    void update_voxel_unit();
  };

  struct grid_visualiser {
    gl_shader m_visual_shader;
    gl_shader m_compute_instances_shader;
    VAO m_texel_shape;
    GLuint m_instance_matrices_ssbo;
    int m_texel_resolution;
    int m_total_invocations;
    unsigned int m_index_count;
    float m_debug_scale = 1.0f;
    glm::vec3 m_debug_position_offset;

    void dispatch_draw(grid &vg, camera &cam);
  };

  static grid create_grid(glm::ivec3 resolution, aabb bb);
  static grid_visualiser
  create_grid_visualiser(grid &vg, gl_shader &visualisation_shader,
                         gl_shader &compute_matrices_shader,
                         int texel_resolution = 8);
};
} // namespace gem