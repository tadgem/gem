#pragma once

#include "glm.hpp"
#include "gtc/quaternion.hpp"

class scene;

struct transform
{
	glm::vec3 m_position	{ 0.0, 0.0, 0.0 };
	glm::vec3 m_euler		{ 0.0, 0.0, 0.0 };
	glm::vec3 m_scale		{ 1.0, 1.0, 1.0 };
	glm::quat m_rotation	= glm::quat();

	glm::mat4 m_model		= glm::mat4(1.0);
	glm::mat4 m_last_model	= glm::mat4(1.0);

	glm::mat3 m_normal_matrix;

	static void update_transforms(scene& current_scene);
};