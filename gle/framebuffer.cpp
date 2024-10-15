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


void framebuffer::add_colour_attachment(GLenum attachment_index, uint32_t width, uint32_t height, GLenum internal_format, GLenum format, GLenum filter, GLenum pixel_format)
{
	gl_handle textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, internal_format, pixel_format, NULL);
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

void framebuffer::add_depth_attachment_sampler_friendly(uint32_t width, uint32_t height, GLenum format)
{
	gl_handle depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	m_depth_attachment = depthMap;
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

framebuffer framebuffer::create(glm::vec2 resolution, std::vector<attachment_info> attachments, bool add_depth)
{
	GLenum attachment = GL_COLOR_ATTACHMENT0;
	framebuffer fb {};
	fb.bind();
	for (auto& info : attachments)
	{
		fb.add_colour_attachment(attachment, resolution.x, resolution.y, info.m_internal_format, info.m_format, info.m_filter, info.m_pixel_format);
		attachment++;
	}
	if (add_depth)
	{
		fb.add_depth_attachment_sampler_friendly(resolution.x, resolution.y, GL_DEPTH24_STENCIL8);
	}
	fb.check();
	fb.unbind();
	return fb;
}

void framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
}
