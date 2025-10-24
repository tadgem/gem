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
  enum projection_type { orthographic, perspective };

  glm::mat4 m_view;
  glm::mat4 m_proj;
  glm::mat4 m_last_vp;

  glm::vec3 m_pos, m_euler, m_forward, m_right, m_up;
  float m_fov = 60.0f, m_aspect = 1.666f, m_near = 0.01f, m_far = 300.0f;

  FrustumPlanes m_frustum_planes;

  projection_type m_projection_type = projection_type::perspective;

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