#pragma once

#include "glm.hpp"
#include "gtc/quaternion.hpp"

struct transform
{
	glm::vec3 m_position;
	glm::vec3 m_euler;
	glm::vec3 m_scale;
	glm::quat m_rotation;

	glm::mat4 m_model;
	glm::mat4 m_last_model;
};