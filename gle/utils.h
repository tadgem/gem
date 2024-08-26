#pragma once

#include <string>
#include "glm.hpp"
class utils
{
public:

	static std::string	load_string_from_path(const std::string& path);
	static glm::quat	get_quat_from_euler(glm::vec3 euler);
	static glm::mat4	get_model_matrix(glm::vec3 position, glm::vec3 euler, glm::vec3 scale);
	static void			validate_euler_angles(glm::vec3& input);

};