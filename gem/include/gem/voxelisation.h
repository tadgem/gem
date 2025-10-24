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
    GLShader m_visual_shader;
    GLShader m_compute_instances_shader;
    VAO m_texel_shape;
    GLuint m_instance_matrices_ssbo;
    int m_texel_resolution;
    int m_total_invocations;
    unsigned int m_index_count;
    float m_debug_scale = 1.0f;
    glm::vec3 m_debug_position_offset;

    void Draw(Grid &vg, Camera &cam);
  };

  static Grid CreateGrid(glm::ivec3 resolution, AABB bb);
  static GridVisualizer
  CreateGridVisualizer(Grid &vg, GLShader &visualisation_shader,
                         GLShader &compute_matrices_shader,
                         int texel_resolution = 8);
};
} // namespace gem