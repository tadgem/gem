#pragma once
#include "gem/mesh.h"

namespace gem {

class asset_manager;

class shapes {
public:
  inline static VAO s_screen_quad;
  inline static VAO s_cube_pos_only;

  inline static VAO             s_cube_vao;
  inline static mesh            s_cube_mesh;

  inline static VAO             s_sphere_vao;
  inline static mesh            s_sphere_mesh;

  inline static VAO             s_cylinder_vao;
  inline static mesh            s_cylinder_mesh;

  inline static VAO             s_cone_vao;
  inline static mesh            s_cone_mesh;

  inline static VAO             s_torus_vao;
  inline static mesh            s_torus_mesh;

  static void init_built_in_assets(asset_manager& am);

  static VAO gen_cube_instanced_vao(std::vector<glm::mat4> &matrices,
                                    std::vector<glm::vec3> &uvs);
};
} // namespace gem