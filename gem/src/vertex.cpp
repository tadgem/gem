#include "gem/vertex.h"
#include "gem/profile.h"

void VAO::use()
{
	ZoneScoped;
	glBindVertexArray(m_vao_id);
}
void VAO::free() {
	ZoneScoped;
    if(m_ibo > 0)
    {
        glDeleteBuffers(1, &m_ibo);
    }
    if(!m_vbos.empty()) {
        glDeleteBuffers(m_vbos.size(), m_vbos.data());
    }
    glDeleteVertexArrays(1, &m_vao_id);
}

void vao_builder::begin()
{
	ZoneScoped;
	m_offset_counter = 0;
	m_ibo = 0;
	m_vbos.clear();
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
}

void vao_builder::add_index_buffer(uint32_t* data, uint32_t data_count)
{
	ZoneScoped;
	gl_handle ibo;
	glGenBuffers(1, &ibo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * data_count, data, GL_STATIC_DRAW);
	m_ibo = ibo;
}

void vao_builder::add_index_buffer(std::vector<uint32_t> data)
{
	ZoneScoped;
	add_index_buffer(data.data(), data.size());
}

void vao_builder::add_vertex_attribute(uint32_t binding, uint32_t total_vertex_size, uint32_t num_elements, uint32_t element_size, GLenum primitive_type)
{
	ZoneScoped;
	glVertexAttribPointer(binding, num_elements, primitive_type, GL_FALSE, total_vertex_size, (void*)m_offset_counter);
	glEnableVertexAttribArray(binding);
	// glBindBuffer(GL_ARRAY_BUFFER, m_vbos.back());
	m_offset_counter += num_elements * element_size;
}

VAO vao_builder::build()
{
	ZoneScoped;
	return VAO{ m_vao, m_ibo, m_vbos };
}

