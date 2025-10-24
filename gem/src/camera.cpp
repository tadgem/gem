#define GLM_ENABLE_EXPERIMENTAL
#include "gem/camera.h"
#include "gem/backend.h"
#include "gem/input.h"
#include "gem/profile.h"
#include "gem/utils.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"

namespace gem {

void Camera::Update(glm::vec2 screen_dim) {
  ZoneScoped;
  m_last_vp = m_proj * m_view;
  glm::mat4 rotate = GetRotationMatrix();
  glm::mat4 translate = glm::mat4(1.0f);
  translate = glm::translate(translate, -m_pos);

  m_view = rotate * translate;
  m_aspect = screen_dim.x / screen_dim.y;

  switch (m_projection_type) {
  case perspective:
    m_proj = glm::perspective(glm::radians(m_fov), screen_dim.x / screen_dim.y,
                              m_near, m_far);
    break;
  case orthographic:
    // todo
    break;
  }

  m_frustum_planes.m_planes = Utils::GetPlanesFromViewProjectionMatrix(m_proj * m_view);
}

glm::mat4 Camera::GetRotationMatrix() {
  ZoneScoped;
  glm::quat qPitch =
      glm::angleAxis(glm::radians(-m_euler.x), glm::vec3(1, 0, 0));
  glm::quat qYaw = glm::angleAxis(glm::radians(m_euler.y), glm::vec3(0, 1, 0));
  // omit roll
  glm::quat Rotation = qPitch * qYaw;
  Rotation = glm::normalize(Rotation);
  return glm::mat4_cast(Rotation);
}

void DebugCameraController::Update(glm::vec2 screen_dim, Camera &cam) {
  ZoneScoped;
  if (!Input::GetMouseButton(MouseButton::right) && !show_mouse) {
    SDL_ShowCursor();
    show_mouse = true;
    return;
  }

  if (!Input::GetMouseButton(MouseButton::right)) {
    return;
  }

  if (show_mouse) {
    show_mouse = false;
    SDL_HideCursor();
  }

  float speed = movement_speed;

  if (Input::GetKey(KeyboardKey::left_shift)) {
    speed *= 3.0f;
  }

  glm::quat q_pitch =
      glm::angleAxis(glm::radians(-cam.m_euler.x), glm::vec3(1, 0, 0));
  glm::quat q_yaw =
      glm::angleAxis(glm::radians(cam.m_euler.y), glm::vec3(0, 1, 0));
  glm::quat rotation = q_pitch * q_yaw;

  glm::vec3 forward = Utils::GetForwardFromQuat(rotation);
  glm::vec3 right = Utils::GetRightFromQuat(rotation);
  glm::vec3 up = Utils::GetUpFromQuat(rotation);
  float frame_time = GPUBackend::Selected()->GetFrameTime();

  cam.m_forward = forward;
  cam.m_right = right;
  cam.m_up = up;

  if (Input::GetKey(KeyboardKey::w)) {
    cam.m_pos += forward * speed * frame_time;
  }

  if (Input::GetKey(KeyboardKey::s)) {
    cam.m_pos -= forward * movement_speed * frame_time;
  }

  if (Input::GetKey(KeyboardKey::a)) {
    cam.m_pos -= right * movement_speed * frame_time;
  }

  if (Input::GetKey(KeyboardKey::d)) {
    cam.m_pos += right * movement_speed * frame_time;
  }

  if (Input::GetKey(KeyboardKey::e)) {
    cam.m_pos += up * movement_speed * frame_time;
  }

  if (Input::GetKey(KeyboardKey::q)) {
    cam.m_pos -= up * movement_speed * frame_time;
  }

  glm::vec2 mouse_velocity = Input::GetMouseVelocity() / screen_dim;
  if (glm::abs(mouse_velocity.x) > deadzone) {
    cam.m_euler.y +=
        mouse_velocity.x * rotational_speed * rotational_factor * frame_time;
  }
  if (glm::abs(mouse_velocity.y) > deadzone) {
    cam.m_euler.x -=
        mouse_velocity.y * rotational_speed * rotational_factor * frame_time;
  }
}
} // namespace gem