#include "camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtc/quaternion.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"
#include "input.h"
#include "utils.h"
#include "gl.h"

void camera::update(glm::vec2 screen_dim)
{
    glm::quat qPitch = glm::angleAxis(glm::radians(-m_euler.x), glm::vec3(1, 0, 0));
    glm::quat qYaw = glm::angleAxis(glm::radians(m_euler.y), glm::vec3(0, 1, 0));
    // omit roll
    glm::quat Rotation = qPitch * qYaw;
    Rotation = glm::normalize(Rotation);
    glm::mat4 rotate = glm::mat4_cast(Rotation);
    glm::mat4 translate = glm::mat4(1.0f);
    translate = glm::translate(translate, -m_pos);

    m_view = rotate * translate;
    m_aspect = screen_dim.x / screen_dim.y;

    switch (m_projection_type )
    {
    case perspective:
        m_proj = glm::perspectiveFov(glm::radians(m_fov), screen_dim.x, screen_dim.y, m_near, m_far);
        break;
    case orthographic:
        // todo
        break;
    }
}

void debug_camera_controller::update(glm::vec2 screen_dim, camera& cam)
{
    if (!input::get_mouse_button(mouse_button::right))
    {
        SDL_ShowCursor(SDL_ENABLE);
        return;
    }

    SDL_ShowCursor(SDL_DISABLE);

    glm::quat q_pitch = glm::angleAxis(glm::radians(-cam.m_euler.x), glm::vec3(1, 0, 0));
    glm::quat q_yaw = glm::angleAxis(glm::radians(cam.m_euler.y), glm::vec3(0, 1, 0));
    glm::quat rotation = q_pitch * q_yaw;

    glm::vec3 forward = utils::get_forward_from_quat(rotation);
    glm::vec3 right = utils::get_right_from_quat(rotation);
    glm::vec3 up = utils::get_up_from_quat(rotation);
    float frame_time = engine::get_frame_time();

    if (input::get_keyboard_key(keyboard_key::w))
    {
        cam.m_pos += forward * movement_speed * frame_time;
    }

    if (input::get_keyboard_key(keyboard_key::s))
    {
        cam.m_pos -= forward * movement_speed * frame_time;
    }

    if (input::get_keyboard_key(keyboard_key::a))
    {
        cam.m_pos -= right * movement_speed * frame_time;
    }

    if (input::get_keyboard_key(keyboard_key::d))
    {
        cam.m_pos += right * movement_speed * frame_time;
    }

    if (input::get_keyboard_key(keyboard_key::e))
    {
        cam.m_pos += up * movement_speed * frame_time;
    }

    if (input::get_keyboard_key(keyboard_key::q))
    {
        cam.m_pos -= up * movement_speed * frame_time;
    }

    glm::vec2 mouse_velocity = input::get_mouse_velocity() / screen_dim;
    if (glm::abs(mouse_velocity.x) > deadzone) {
        cam.m_euler.y += mouse_velocity.x * rotational_speed * rotational_factor * frame_time;
    }
    if (glm::abs(mouse_velocity.y) > deadzone) {
        cam.m_euler.x -= mouse_velocity.y * rotational_speed * rotational_factor * frame_time;
    }

}