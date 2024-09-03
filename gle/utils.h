#pragma once

#include <string>
#include "glm.hpp"
#include "shape.h"

class utils
{
public:

	static std::string	load_string_from_path(const std::string& path);
	static glm::quat	get_quat_from_euler(glm::vec3 euler);
	static glm::mat4	get_model_matrix(glm::vec3 position, glm::vec3 euler, glm::vec3 scale);
	static glm::mat3	get_normal_matrix(glm::mat4 model);
	static glm::vec3	cart_to_spherical(glm::vec3 normal);
	static glm::vec3	spherical_to_cart(glm::vec3 spherical);
	static aabb			transform_aabb(aabb& in, glm::mat4& model);
	static void			validate_euler_angles(glm::vec3& input);

};