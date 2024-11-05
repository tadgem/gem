#pragma once
#include "gem/shader.h"
#include "gem/shape.h"
#include "gem/texture.h"
#include "gem/camera.h"

namespace gem {

class voxel {
public:
  struct grid {
    texture     voxel_texture; // 3D Texture (Voxel Data)
    glm::ivec3  resolution;
    glm::vec3   voxel_unit; // scale of each texel
    glm::vec3   aabb_dim {150.0, 100.0, 150.0};
    aabb        current_bounding_box;
    aabb        previous_bounding_box;

    void update_voxel_unit();
    void update_grid_position(camera& cam);
  };

  struct grid_visualiser {
    shader        m_visual_shader;
    VAO           m_texel_shape;
    int           m_texel_resolution;
    int           m_total_invocations;
    unsigned int  m_index_count;
    float         m_debug_scale = 1.0f;
    glm::vec3     m_debug_position_offset;

    void dispatch_draw(grid& vg, camera& cam);
  };

  static grid create_grid(glm::ivec3 resolution, aabb bb);
  static grid_visualiser create_grid_visualiser(grid &vg,
                                                shader &vvisualisation_shader,
                                                int texel_resolution = 8);
};
} // namespace gem