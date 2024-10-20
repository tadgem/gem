#pragma once
#include <vector>
#include "GL/glew.h"
#include "alias.h"

struct VAO
{
	gl_handle m_vao_id;
        gl_handle m_ibo =0;
        std::vector<gl_handle> m_vbos;

	void	use();
        void    free();
};

class vao_builder
{
public:

	vao_builder() = default;

	void begin();

	template<typename _Ty>
	void add_vertex_buffer(_Ty* data, uint32_t count)
	{		
		gl_handle vbo;
		glGenBuffers(1, &vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		auto data_size = sizeof(_Ty) * count;
		glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
		m_vbos.push_back(vbo);
	}

	template<typename _Ty>
	void add_vertex_buffer(std::vector<_Ty> data)
	{
		add_vertex_buffer<_Ty>(&data[0], data.size());
	}

	void add_index_buffer(uint32_t* data, uint32_t data_count);
	void add_index_buffer(std::vector<uint32_t> data);

	void add_vertex_attribute(uint32_t binding, uint32_t total_vertex_size, uint32_t num_elements, uint32_t element_size = 4, GLenum primitive_type = GL_FLOAT);

	VAO	 build();

	gl_handle					m_vao;
	gl_handle					m_ibo;
	std::vector<gl_handle>		m_vbos;
	uint32_t					m_offset_counter;

};
