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

void camera::update(glm::vec2 screen_dim) {
  ZoneScoped;
  m_last_vp = m_proj * m_view;
  glm::mat4 rotate = get_rotation_matrix();
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

  m_frustum_planes.m_planes = utils::get_planes_from_view_proj(m_proj * m_view);
}

glm::mat4 camera::get_rotation_matrix() {
  ZoneScoped;
  glm::quat qPitch =
      glm::angleAxis(glm::radians(-m_euler.x), glm::vec3(1, 0, 0));
  glm::quat qYaw = glm::angleAxis(glm::radians(m_euler.y), glm::vec3(0, 1, 0));
  // omit roll
  glm::quat Rotation = qPitch * qYaw;
  Rotation = glm::normalize(Rotation);
  return glm::mat4_cast(Rotation);
}

void debug_camera_controller::update(glm::vec2 screen_dim, camera &cam) {
  ZoneScoped;
  if (!input::get_mouse_button(mouse_button::right) && !show_mouse) {
    SDL_ShowCursor();
    show_mouse = true;
    return;
  }

  if (!input::get_mouse_button(mouse_button::right)) {
    return;
  }

  if (show_mouse) {
    show_mouse = false;
    SDL_HideCursor();
  }

  float speed = movement_speed;

  if (input::get_keyboard_key(keyboard_key::left_shift)) {
    speed *= 3.0f;
  }

  glm::quat q_pitch =
      glm::angleAxis(glm::radians(-cam.m_euler.x), glm::vec3(1, 0, 0));
  glm::quat q_yaw =
      glm::angleAxis(glm::radians(cam.m_euler.y), glm::vec3(0, 1, 0));
  glm::quat rotation = q_pitch * q_yaw;

  glm::vec3 forward = utils::get_forward_from_quat(rotation);
  glm::vec3 right = utils::get_right_from_quat(rotation);
  glm::vec3 up = utils::get_up_from_quat(rotation);
  float frame_time = gpu_backend::selected()->get_frame_time();

  cam.m_forward = forward;
  cam.m_right = right;
  cam.m_up = up;

  if (input::get_keyboard_key(keyboard_key::w)) {
    cam.m_pos += forward * speed * frame_time;
  }

  if (input::get_keyboard_key(keyboard_key::s)) {
    cam.m_pos -= forward * movement_speed * frame_time;
  }

  if (input::get_keyboard_key(keyboard_key::a)) {
    cam.m_pos -= right * movement_speed * frame_time;
  }

  if (input::get_keyboard_key(keyboard_key::d)) {
    cam.m_pos += right * movement_speed * frame_time;
  }

  if (input::get_keyboard_key(keyboard_key::e)) {
    cam.m_pos += up * movement_speed * frame_time;
  }

  if (input::get_keyboard_key(keyboard_key::q)) {
    cam.m_pos -= up * movement_speed * frame_time;
  }

  glm::vec2 mouse_velocity = input::get_mouse_velocity() / screen_dim;
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