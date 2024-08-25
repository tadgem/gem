#include "camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtc/quaternion.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"
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
