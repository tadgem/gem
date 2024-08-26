#pragma once
#include "vertex.h"

class framebuffer
{
public:
	framebuffer();

	void	bind();
	void	unbind();
	void	cleanup();

	void	add_colour_attachment(GLenum attachment_index, uint32_t width, uint32_t height, GLenum format, GLenum filter = GL_LINEAR, GLenum pixel_format = GL_UNSIGNED_BYTE);
	void	add_depth_attachment(uint32_t width, uint32_t height, GLenum format = GL_DEPTH24_STENCIL8);
	void	check();

	gl_handle				m_handle;
	std::vector<gl_handle>	m_colour_attachments;
	gl_handle				m_depth_attachment;
};