#pragma once

#include <string>
#include "glm.hpp"
#include "shape.h"
#include "alias.h"

class utils
{
public:

	[[always_inline]] static std::string		load_string_from_path(const std::string& path);
	[[always_inline]] static std::vector<u8>	load_binary_from_path(const std::string& path);
	[[always_inline]] static glm::quat			get_quat_from_euler(glm::vec3 euler);
	[[always_inline]] static glm::mat4			get_model_matrix(glm::vec3 position, glm::vec3 euler, glm::vec3 scale);
	[[always_inline]] static glm::mat3			get_normal_matrix(glm::mat4 model);
	[[always_inline]] static glm::vec3			cart_to_spherical(glm::vec3 normal);
	[[always_inline]] static glm::vec3			spherical_to_cart(glm::vec3 spherical);
	[[always_inline]] static glm::vec3			get_forward(glm::vec3 euler);
	[[always_inline]] static glm::vec3			get_right(glm::vec3 euler);
	[[always_inline]] static glm::vec3			get_up(glm::vec3 euler);
	[[always_inline]] static glm::vec3			get_forward_from_quat(glm::quat rot);
	[[always_inline]] static glm::vec3			get_right_from_quat(glm::quat rot);
	[[always_inline]] static glm::vec3			get_up_from_quat(glm::quat rot);
	[[always_inline]] static glm::vec3			get_mouse_world_pos(glm::vec2 mouse_pos, glm::vec2 resolution, glm::mat4& proj, glm::mat4& view);
	[[always_inline]] static float				round_up(float value, int decimal_places);
	[[always_inline]] static aabb				transform_aabb(aabb& in, glm::mat4& model);
	[[always_inline]] static void				validate_euler_angles(glm::vec3& input);

};