#pragma once
#include "gem/alias.h"
#include "glm.hpp"

namespace gem {

struct DirectionalLight {
  glm::vec3 direction;
  glm::vec3 colour;
  glm::mat4 light_space_matrix;
  float intensity = 1.0f;
};

struct PointLight {
  glm::vec3 position;
  glm::vec3 colour;
  float radius;
  float intensity = 1.0f;
};
} // namespace gem