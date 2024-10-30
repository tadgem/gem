#pragma once

#include <string>
#include <array>
#include <future>
#include "glm.hpp"
#include "gem/shape.h"
#include "gem/alias.h"

class utils
{
public:

	static std::string					load_string_from_path(const std::string& path);
	static void							save_string_to_path(const std::string& path, const std::string& str);
	static std::vector<u8>				load_binary_from_path(const std::string& path);
	static void							save_binary_to_path(const std::string& path, std::vector<u8>& bytes);
	static glm::quat					get_quat_from_euler(glm::vec3 euler);
	static glm::mat4					get_model_matrix(glm::vec3 position, glm::vec3 euler, glm::vec3 scale);
	static glm::mat3					get_normal_matrix(glm::mat4 model);
	static glm::vec3					cart_to_spherical(glm::vec3 normal);
	static glm::vec3					spherical_to_cart(glm::vec3 spherical);
	static glm::vec3					get_forward(glm::vec3 euler);
	static glm::vec3					get_right(glm::vec3 euler);
	static glm::vec3					get_up(glm::vec3 euler);
	static glm::vec3					get_forward_from_quat(glm::quat rot);
	static glm::vec3					get_right_from_quat(glm::quat rot);
	static glm::vec3					get_up_from_quat(glm::quat rot);
	static glm::vec3					get_mouse_world_pos(glm::vec2 mouse_pos, glm::vec2 resolution, glm::mat4& proj, glm::mat4& view);
	static float						round_up(float value, int decimal_places);
	static aabb							transform_aabb(aabb& in, glm::mat4& model);
	static void							validate_euler_angles(glm::vec3& input);
	static glm::vec3					screen_to_world_ray(glm::vec3 eye_pos, glm::vec2 mouse_pos, glm::vec2 screen_dim, glm::mat4 inverse_view, glm::mat4 inverse_proj);
	static glm::vec3					screen_to_world_pos(glm::vec2 mouse_pos, glm::vec2 screen_dim, glm::mat4 inverse_view, glm::mat4 inverse_proj);
	static std::array<glm::vec4, 6>		get_planes_from_view_proj(glm::mat4 view_proj);
	static std::string					get_directory_from_path(const std::string& path);

	// template utils
	template<typename _Ty>
	static bool							is_future_ready(std::future<_Ty> const& o) {
		return o.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}
};