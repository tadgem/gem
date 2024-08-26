#pragma once
#include "glm.hpp"

struct camera
{
	enum projection_type
	{
		orthographic,
		perspective
	};

	glm::mat4 m_view;
	glm::mat4 m_proj;

	glm::vec3	m_pos, m_euler;
	float		m_fov = 60.0f, m_aspect = 1.666f, m_near = 0.01f, m_far = 500.0f;

	projection_type m_projection_type = projection_type::perspective;

	void update(glm::vec2 screen_dim);
};