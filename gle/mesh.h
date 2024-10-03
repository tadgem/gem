#pragma once

#include "vertex.h"
#include "shape.h"


struct mesh
{
	VAO				m_vao;
	uint32_t		m_index_count;
	aabb			m_original_aabb;
	aabb			m_transformed_aabb;
	uint32_t		m_material_index;

	void draw()
	{
		m_vao.use();
		glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, 0);
	}
};
