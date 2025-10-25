#pragma once
#include "gem/AABB.h"
#include "gem/camera.h"
#include "gem/gl/gl_shader.h"
#include "gem/texture.h"

namespace gem {

class Voxel {
public:
  struct Grid {
    Texture voxel_texture; // 3D Texture (Voxel Data)
    glm::ivec3 resolution;
    glm::vec3 voxel_unit; // dimensons of each texel
    glm::vec3 aabb_dim{200.0, 200.0, 200.0};
    AABB current_bounding_box;
    AABB previous_bounding_box;

    void UpdateVoxelUnit();
  };

  struct GridVisualizer {
    GLShader visualization_shader;
    GLShader compute_instance_matrices_shader;
    VAO voxel_unit_shape;
    GLuint instance_matrices_ssbo;
    int texel_resolution_cubed;
    int required_draw_calls;
    unsigned int index_count;
    float debug_scale = 1.0f;
    glm::vec3 debug_position_offset;

    void Draw(Grid &vg, Camera &cam);
  };

  static Grid CreateGrid(glm::ivec3 resolution, AABB bb);
  static GridVisualizer
  CreateGridVisualizer(Grid &vg, GLShader &visualisation_shader,
                         GLShader &compute_matrices_shader,
                         int texel_resolution = 8);
};
} // namespace gem