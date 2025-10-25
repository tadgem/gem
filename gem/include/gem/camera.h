#pragma once
#include "gem/dbg_memory.h"
#include "glm.hpp"
#include <array>

namespace gem {

struct FrustumPlanes {
  std::array<glm::vec4, 6> m_planes; // L,R,B,T,N,F

  GEM_IMPL_ALLOC(FrustumPlanes)
};

struct Camera {
  enum ProjectionType { kOrthographic, kPerspective };

  glm::mat4 view_matrix;
  glm::mat4 proj_matrix;
  glm::mat4 prev_view_proj;

  glm::vec3 position, euler, forward, right, up;
  float fov = 60.0f, aspect = 1.666f, near_plane = 0.01f, far_plane = 300.0f;

  FrustumPlanes frustum_planes;

  ProjectionType projection_type = ProjectionType::kPerspective;

  void Update(glm::vec2 screen_dim);

  glm::mat4 GetRotationMatrix();

  GEM_IMPL_ALLOC(Camera)
};

struct DebugCameraController {
  void Update(glm::vec2 screen_dim, Camera &cam);

  float movement_speed = 10.0f;
  float deadzone = 0.002f;
  float rotational_speed = 45.0f;
  static constexpr float rotational_factor = 360.0f;

  bool show_mouse = true;

  GEM_IMPL_ALLOC(DebugCameraController)
};
} // namespace gem