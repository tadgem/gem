#pragma once
#include <vector>
#include "GL/glew.h"
struct VAO
{
	unsigned int m_vao_id;

	void	use();
};

class vao_builder
{
public:

	vao_builder() = default;

	void begin();

	template<typename _Ty>
	void add_buffer(float* data, uint32_t count, uint32_t num_float_elements_per_vertex = 2)
	{		
		unsigned int vbo;
		glGenBuffers(1, &vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_Ty) * count, data, GL_STATIC_DRAW);
	}

	void add_vertex_attribute(uint32_t binding, uint32_t total_vertex_size, uint32_t num_elements, uint32_t element_size = 4, GLenum primitive_type = GL_FLOAT);

	VAO	 build();
private:

	unsigned int				m_vao;
	std::vector<unsigned int>	m_vbos;
	uint32_t					m_offset_counter;
};
