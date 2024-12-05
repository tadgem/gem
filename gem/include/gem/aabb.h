#pragma once
#include "gem/dbg_memory.h"
#include "glm/glm.hpp"

namespace gem {
struct aabb {
  glm::vec3 min;
  glm::vec3 max;

  GEM_IMPL_ALLOC(aabb)
};
} // namespace gem