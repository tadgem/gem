#pragma once
#include "glm/glm.hpp"
#include "gem/dbg_memory.h"

namespace gem {
  struct aabb {
    glm::vec3 min;
    glm::vec3 max;

    GEM_IMPL_ALLOC(aabb)
  };
}