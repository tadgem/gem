#pragma once
#include "alias.h"
#include "GL/glew.h"
#include "glm.hpp"
#include <cstdint>
#include <vector>
#include <array>
class framebuffer
{
public:
	framebuffer();

	void	bind();
	void	unbind();
	void	cleanup();

	void	add_colour_attachment(GLenum attachment_index, uint32_t width, uint32_t height, GLenum format, GLenum filter = GL_LINEAR, GLenum pixel_format = GL_UNSIGNED_BYTE);
	void	add_depth_attachment(uint32_t width, uint32_t height, GLenum format = GL_DEPTH24_STENCIL8);
	void	add_depth_attachment_sampler_friendly(uint32_t width, uint32_t height, GLenum format = GL_DEPTH24_STENCIL8);
	void	check();

	template<typename _Ty, size_t w, size_t h>
	std::array<_Ty, w * h> read_pixels(u32 x, u32 y, u32 attachment_index, GLenum pixel_format, GLenum pixel_type)
	{
		std::array<_Ty, w * h> out_data{};
		bind();
		glReadPixels(x, y, w, h, pixel_format, pixel_type, &out_data[0]);
		unbind();
		return out_data;
	}

	struct attachment_info
	{
		GLenum m_format;
		GLenum m_filter;
		GLenum m_pixel_format;
	};

	static	framebuffer		create(glm::vec2 resolution, std::vector<attachment_info> colour_attachments, bool add_depth = true);

	gl_handle				m_handle;
	std::vector<gl_handle>	m_colour_attachments;
	gl_handle				m_depth_attachment;
	uint32_t				m_width, m_height;
};