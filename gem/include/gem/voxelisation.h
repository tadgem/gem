#pragma once
#include "gem/shader.h"
#include "gem/aabb.h"
#include "gem/texture.h"
#include "gem/camera.h"

namespace gem {

class voxel {
public:
  struct grid {
    texture     voxel_texture; // 3D Texture (Voxel Data)
    texture     history_voxel_texture;
    glm::ivec3  resolution;
    glm::vec3   voxel_unit; // scale of each texel
    glm::vec3   aabb_dim {200.0, 100.0, 200.0};
    aabb        current_bounding_box;
    aabb        previous_bounding_box;

    void update_voxel_unit();
    void update_grid_history(camera &cam, bool force = false);
  };

  struct grid_visualiser {
    shader        m_visual_shader;
    shader        m_compute_instances_shader;
    VAO           m_texel_shape;
    GLuint        m_instance_matrices_ssbo;
    int           m_texel_resolution;
    int           m_total_invocations;
    unsigned int  m_index_count;
    float         m_debug_scale = 1.0f;
    glm::vec3     m_debug_position_offset;

    void dispatch_draw(grid& vg, camera& cam);
  };

  static grid create_grid(glm::ivec3 resolution, aabb bb);
  static grid_visualiser create_grid_visualiser(grid &vg,
                                                shader &visualisation_shader,
                                                shader &compute_matrices_shader,
                                                int texel_resolution = 8);
};
} // namespace gem