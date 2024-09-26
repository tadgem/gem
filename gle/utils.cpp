#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#define GLM_ENABLE_EXPERIMENTAL
#include "gtc/quaternion.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"
#include "gtx/matrix_decompose.hpp"
std::string utils::load_string_from_path(const std::string& path)
{
    std::ifstream in(path);
    std::stringstream stream;
    if (!in.is_open())
    {
        return "";
    }

    stream << in.rdbuf();
    return stream.str();
}

glm::quat utils::get_quat_from_euler(glm::vec3 euler)
{
    glm::vec3 eulerRadians = glm::vec3(glm::radians(euler.x), glm::radians(euler.y), glm::radians(euler.z));
    glm::quat xRotation = glm::angleAxis(eulerRadians.x, glm::vec3(1, 0, 0));
    glm::quat yRotation = glm::angleAxis(eulerRadians.y, glm::vec3(0, 1, 0));
    glm::quat zRotation = glm::angleAxis(eulerRadians.z, glm::vec3(0, 0, 1));

    return zRotation * yRotation * xRotation;

}

glm::mat4 utils::get_model_matrix(glm::vec3 position, glm::vec3 euler, glm::vec3 scale)
{
    glm::mat4 modelMatrix = glm::mat4(1.0);

    modelMatrix = glm::translate(modelMatrix, position);
    validate_euler_angles(euler);
    glm::quat rot = get_quat_from_euler(euler);
    glm::mat4 localRotation = glm::mat4_cast(rot);
    glm::mat4 localScale = glm::mat4(1.0);
    localScale = glm::scale(localScale, scale);
    
    return modelMatrix * localRotation * localScale;
}

glm::mat3 utils::get_normal_matrix(glm::mat4 model)
{
    return glm::transpose(glm::inverse(glm::mat3(model)));
}

glm::vec3 utils::cart_to_spherical(glm::vec3 normal)
{
    float p = glm::sqrt(glm::pow(normal.x, 2) + glm::pow(normal.y, 2) + glm::pow(normal.z, 2));
    float theta = glm::acos((normal.y) / (normal.x));
    if (std::isnan(theta))
    {
        theta = 0.0f;
    }
    float phi = glm::acos((normal.z) / p);
    return glm::vec3(p, theta, phi);
}

glm::vec3 utils::spherical_to_cart(glm::vec3 spherical)
{
    // p * sin-phi * cos-theta
    float x = spherical.x * glm::sin(spherical.z) * glm::cos(spherical.y);
    // p * sin-phi * sin-theta
    float y = spherical.x * glm::sin(spherical.z) * glm::sin(spherical.y);
    // p * cos-phi
    float z = spherical.x * glm::cos(spherical.z);
    return glm::vec3(round_up(y, 4),round_up(x, 4), round_up(z, 4));
}

glm::vec3 utils::get_forward(glm::vec3 euler)
{
    float pitch = glm::radians(euler.x);
    float yaw = glm::radians(euler.y);
    glm::vec3 forward;
    forward.x = glm::cos(pitch) * glm::sin(yaw);
    forward.y = -glm::sin(pitch);
    forward.z = glm::cos(pitch) * glm::cos(yaw);
    return forward;
}

glm::vec3 utils::get_right(glm::vec3 euler)
{
    glm::vec3 right;
    right.x = glm::cos(glm::radians(euler.y));
    right.y = 0.0f;
    right.z = -glm::sin(glm::radians(euler.y));
    return right;
}

glm::vec3 utils::get_up(glm::vec3 euler)
{
    glm::vec3 up;
    glm::vec3 eulerRadians = glm::radians(euler);
    up.x = glm::sin(eulerRadians.x) * glm::sin(eulerRadians.y);
    up.y = glm::cos(eulerRadians.x);
    up.z = glm::sin(eulerRadians.x) * glm::cos(eulerRadians.y);
    return up;
}

glm::vec3 utils::get_forward_from_quat(glm::quat rot)
{
    return glm::rotate(glm::inverse(rot), glm::vec3(0.0, 0.0, -1.0));
}

glm::vec3 utils::get_right_from_quat(glm::quat rot)
{
    return glm::rotate(glm::inverse(rot), glm::vec3(1.0, 0.0, 0.0));
}

glm::vec3 utils::get_up_from_quat(glm::quat rot)
{
    return glm::vec3(0.0, 1.0, 0.0);
}

float utils::round_up(float value, int decimal_places)
{
    const float multiplier = std::pow(10.0, decimal_places);
    return std::ceil(value * multiplier) / multiplier;
}

aabb utils::transform_aabb(aabb& box, glm::mat4& M)
{
    glm::vec3 corners[8];
    corners[0] = box.min;
    corners[1] = glm::vec3(box.min.x, box.max.y, box.min.z);
    corners[2] = glm::vec3(box.min.x, box.max.y, box.max.z);
    corners[3] = glm::vec3(box.min.x, box.min.y, box.max.z);
    corners[4] = glm::vec3(box.max.x, box.min.y, box.min.z);
    corners[5] = glm::vec3(box.max.x, box.max.y, box.min.z);
    corners[6] = box.max;
    corners[7] = glm::vec3(box.max.x, box.min.y, box.max.z);

    // transform the first corner
    glm::vec3 tmin = glm::vec3(M * glm::vec4(corners[0], 1.0));
    glm::vec3 tmax = tmin;

    // transform the other 7 corners and compute the result AABB
    for (int i = 1; i < 8; i++)
    {
        glm::vec3 point = glm::vec3(M * glm::vec4(corners[i], 1.0));

        tmin = min(tmin, point);
        tmax = max(tmax, point);
    }

    aabb rbox;

    rbox.min = tmin;
    rbox.max = tmax;

    return rbox;
}

void utils::validate_euler_angles(glm::vec3& input)
{
    if (input.x <= 0.0001f && input.x >= 0.0001f) {
        input.x += 0.0001f;
    }

    if (input.y <= 0.0001f && input.y >= 0.0001f) {
        input.y += 0.0001f;
    }

    if (input.z <= 0.0001f && input.z >= 0.0001f) {
        input.z += 0.0001f;
    }
}
