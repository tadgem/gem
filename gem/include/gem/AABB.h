#pragma once
#include "gem/dbg_memory.h"
#include "glm/glm.hpp"

namespace gem {
struct AABB {
  glm::vec3 m_min;
  glm::vec3 m_max;

  void Scale(const glm::vec3& scale) {
    m_min *= scale;
    m_max *= scale;
  }

  GEM_IMPL_ALLOC(AABB)
};
} // namespace gem