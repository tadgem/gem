#include "vertex.h"


void VAO::use()
{
	glBindVertexArray(m_vao_id);
}

void vao_builder::begin()
{
	m_offset_counter = 0;
	m_vbos.clear();
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
}

void vao_builder::add_vertex_attribute(uint32_t binding, uint32_t total_vertex_size, uint32_t num_elements, uint32_t element_size, GLenum primitive_type)
{

	glVertexAttribPointer(binding, num_elements, primitive_type, GL_FALSE, total_vertex_size, (void*)m_offset_counter);
	glEnableVertexAttribArray(binding);

	m_offset_counter += num_elements * element_size;
}

VAO vao_builder::build()
{
	return VAO{ m_vao };
}

