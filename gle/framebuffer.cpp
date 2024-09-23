#include "framebuffer.h"
#include <iostream>

framebuffer::framebuffer()
{
	glGenFramebuffers(1, &m_handle);
}

void framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::cleanup()
{
	glDeleteFramebuffers(1, &m_handle);
}


void framebuffer::add_colour_attachment(GLenum attachment_index, uint32_t width, uint32_t height, GLenum format, GLenum filter, GLenum pixel_format)
{
	gl_handle textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, pixel_format, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_index, GL_TEXTURE_2D, textureColorbuffer, 0);
	m_colour_attachments.push_back(textureColorbuffer);
	m_width = width;
	m_height = height;
}

void framebuffer::add_depth_attachment(uint32_t width, uint32_t height, GLenum format)
{
	gl_handle rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	m_depth_attachment = rbo;
	m_width = width;
	m_height = height;
}

void framebuffer::check()
{
	auto zero = GL_COLOR_ATTACHMENT0;
	std::vector<unsigned int> attachments;

	for (auto& a : m_colour_attachments)
	{
		attachments.push_back(zero);
		zero++;
	}

	if (attachments.empty())
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		glDrawBuffers(attachments.size(), attachments.data());
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
}

void framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
}
