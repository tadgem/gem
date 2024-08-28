#pragma once
#include "glm.hpp"
#include "vertex.h"

class shapes
{
public:

	struct aabb
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct screen_quad
	{
		VAO m_vao;
	};

	inline static screen_quad s_screen_quad;
};