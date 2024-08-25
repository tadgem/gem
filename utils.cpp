#include "utils.h"
#include <fstream>
#include <sstream>
#define GLM_ENABLE_EXPERIMENTAL
#include "gtc/quaternion.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"

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
