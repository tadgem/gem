#pragma once
#include "gem/vertex.h"
#include "glm.hpp"

namespace gem {

struct aabb {
  glm::vec3 min;
  glm::vec3 max;
};

class shapes {
public:
  inline static VAO s_screen_quad;
  inline static VAO s_cube_pos_only;

  static VAO gen_cube_instanced_vao(std::vector<glm::mat4> &matrices,
                                    std::vector<glm::vec3> &uvs);
};
} // namespace gem