#include "utils.h"
#include <fstream>
#include <sstream>
#define GLM_ENABLE_EXPERIMENTAL
#include "gtc/quaternion.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"
#include "gtx/matrix_decompose.hpp"
#include "profile.h"

std::string utils::load_string_from_path(const std::string& path)
{
    ZoneScoped;
    std::ifstream in(path);
    std::stringstream stream;
    if (!in.is_open())
    {
        return "";
    }

    stream << in.rdbuf();
    return stream.str();
}

void utils::save_string_to_path(const std::string& path, const std::string& str)
{
    ZoneScoped;
    {
        std::ofstream out(path);

        if (!out.is_open())
        {
            return;
        }

        out << str;
    }
}

std::vector<u8> utils::load_binary_from_path(const std::string& path)
{
    ZoneScoped;
    std::ifstream input(path, std::ios::binary);

    std::vector<u8> bytes((std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));

    input.close();
    return bytes;
}

void utils::save_binary_to_path(const std::string& path, std::vector<u8>& bytes)
{
    ZoneScoped;
    {
        std::ofstream out(path);

        if (!out.is_open())
        {
            return;
        }

        for (u8 b : bytes)
        {
            out << b;
        }

    }
}

glm::quat utils::get_quat_from_euler(glm::vec3 euler)
{
    ZoneScoped;
    glm::vec3 eulerRadians = glm::vec3(glm::radians(euler.x), glm::radians(euler.y), glm::radians(euler.z));
    glm::quat xRotation = glm::angleAxis(eulerRadians.x, glm::vec3(1, 0, 0));
    glm::quat yRotation = glm::angleAxis(eulerRadians.y, glm::vec3(0, 1, 0));
    glm::quat zRotation = glm::angleAxis(eulerRadians.z, glm::vec3(0, 0, 1));

    return zRotation * yRotation * xRotation;

}

glm::mat4 utils::get_model_matrix(glm::vec3 position, glm::vec3 euler, glm::vec3 scale)
{
    ZoneScoped;
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
    ZoneScoped;
    return glm::transpose(glm::inverse(glm::mat3(model)));
}

glm::vec3 utils::cart_to_spherical(glm::vec3 normal)
{
    ZoneScoped;
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
    ZoneScoped;
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
    ZoneScoped;
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
    ZoneScoped;
    glm::vec3 right;
    right.x = glm::cos(glm::radians(euler.y));
    right.y = 0.0f;
    right.z = -glm::sin(glm::radians(euler.y));
    return right;
}

glm::vec3 utils::get_up(glm::vec3 euler)
{
    ZoneScoped;
    glm::vec3 up;
    glm::vec3 eulerRadians = glm::radians(euler);
    up.x = glm::sin(eulerRadians.x) * glm::sin(eulerRadians.y);
    up.y = glm::cos(eulerRadians.x);
    up.z = glm::sin(eulerRadians.x) * glm::cos(eulerRadians.y);
    return up;
}

glm::vec3 utils::get_forward_from_quat(glm::quat rot)
{
    ZoneScoped;
    return glm::rotate(glm::inverse(rot), glm::vec3(0.0, 0.0, -1.0));
}

glm::vec3 utils::get_right_from_quat(glm::quat rot)
{
    ZoneScoped;
    return glm::rotate(glm::inverse(rot), glm::vec3(1.0, 0.0, 0.0));
}

glm::vec3 utils::get_up_from_quat(glm::quat rot)
{
    ZoneScoped;
    return glm::vec3(0.0, 1.0, 0.0);
}

glm::vec3 utils::get_mouse_world_pos(glm::vec2 mouse_pos, glm::vec2 resolution, glm::mat4& proj, glm::mat4& view)
{
    ZoneScoped;
    using namespace glm;
    float x = (2.0f * mouse_pos.x) / resolution.x - 1.0f;
    float y = 1.0f - (2.0f * mouse_pos.y) / resolution.y;
    float z = 1.0f;
    vec3 ray_nds = vec3(x, y, z);
    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    vec4 ray_eye = inverse(proj) * ray_clip;
    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    vec3 ray_wor = (inverse(view) * ray_eye);
    return glm::normalize(ray_wor);
}

float utils::round_up(float value, int decimal_places)
{
    ZoneScoped;
    const float multiplier = std::pow(10.0, decimal_places);
    return std::ceil(value * multiplier) / multiplier;
}

aabb utils::transform_aabb(aabb& box, glm::mat4& M)
{
    ZoneScoped;
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
    ZoneScoped;
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

glm::vec3 utils::screen_to_world_ray(glm::vec3 eye_pos, glm::vec2 mouse_pos, glm::vec2 screen_dim, glm::mat4 inverse_view, glm::mat4 inverse_proj)
{
    ZoneScoped;
    glm::vec3 rayOrigin = utils::screen_to_world_pos(mouse_pos, screen_dim, inverse_view, inverse_proj);;
    return glm::normalize(rayOrigin - eye_pos);
}

glm::vec3 utils::screen_to_world_pos(glm::vec2 mouse_pos, glm::vec2 screen_dim, glm::mat4 inverse_view, glm::mat4 inverse_proj)
{
    ZoneScoped;
    float mouse_x = glm::max(0.0f, mouse_pos.x);
    float ndc_x = (2 * mouse_x - screen_dim.x) / screen_dim.x;
    float ndc_y = (screen_dim.y - 2 * mouse_pos.y) / screen_dim.y;

    glm::vec4 point = inverse_proj * glm::vec4(ndc_x, ndc_y, -1.f, 1.f);
    point = point / point.w;
    glm::vec3 world = inverse_view * point;
    return world;
}

std::array<glm::vec4, 6> utils::get_planes_from_view_proj(glm::mat4 view_proj)
{
    ZoneScoped;
    std::array<glm::vec4, 6> arr = std::array<glm::vec4, 6>();

    for (int i = 4; i--; ) { arr[0][i] = view_proj[i][3] + view_proj[i][0]; }
    for (int i = 4; i--; ) { arr[1][i] = view_proj[i][3] - view_proj[i][0]; }
    for (int i = 4; i--; ) { arr[2][i] = view_proj[i][3] + view_proj[i][1]; }
    for (int i = 4; i--; ) { arr[3][i] = view_proj[i][3] - view_proj[i][1]; }
    for (int i = 4; i--; ) { arr[4][i] = view_proj[i][3] + view_proj[i][2]; }
    for (int i = 4; i--; ) { arr[5][i] = view_proj[i][3] - view_proj[i][2]; }

    return arr;
}

std::string utils::get_directory_from_path(const std::string& path)
{
    ZoneScoped;
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : path.substr(0, pos);
}