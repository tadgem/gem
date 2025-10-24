#pragma once
#include "gem/dbg_memory.h"
#include "glm/glm.hpp"

namespace gem {
struct AABB {
  glm::vec3 min;
  glm::vec3 max;

  void Scale(const glm::vec3& scale) {
    min *= scale;
    max *= scale;
  }

  GEM_IMPL_ALLOC(AABB)
};
} // namespace gem