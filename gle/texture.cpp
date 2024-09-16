#include "texture.h"
#include "texture.h"
#include "texture.h"
#include <iostream>
#include "GL/glew.h"
#include "SOIL/SOIL.h"
#include "gl.h"
#include "stb_image.h"
texture::texture(const std::string& path)
{
	// unsigned char* data = stbi_load(path.c_str(), & m_width, & m_height, &m_num_channels, 0);
	unsigned char* data = SOIL_load_image(path.c_str(), &m_width, &m_height, &m_num_channels, 0);
	if (!data)
	{
		std::cerr << "Failed to load texture at path : " << path << std::endl;
		return;
	}

	// m_handle = SOIL_create_OGL_texture(data, m_width, m_height, m_num_channels, 0, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
	glGenTextures(1, &m_handle);
	glBindTexture(GL_TEXTURE_2D, m_handle);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void texture::bind(GLenum texture_slot, GLenum texture_target)
{
	bind_handle(m_handle, texture_slot, texture_target);
}

void texture::bind_handle(gl_handle handle, GLenum texture_slot, GLenum texture_target)
{
	glActiveTexture(texture_slot);
	glBindTexture(texture_target, handle);
}

texture texture::from_data(unsigned int* data, unsigned int count, int width, int height, int depth, int nr_channels)
{
	texture t{};
	glGenTextures(1, &t.m_handle);
	glBindTexture(GL_TEXTURE_2D, t.m_handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	return t;
}

texture texture::create_3d_texture(glm::ivec3 dim, GLenum format, GLenum pixel_format, GLenum data_type, void* data, GLenum filter, GLenum wrap_mode )
{
	texture t{};
	glAssert(glGenTextures(1, &t.m_handle));
	glAssert(glBindTexture(GL_TEXTURE_3D, t.m_handle));

	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode));	
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 3));

	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	glAssert(glTexImage3D(GL_TEXTURE_3D, 0, pixel_format, dim.x, dim.y, dim.z, 0, format, data_type, data));
	glGenerateTextureMipmap(t.m_handle);
	return t;
}

texture texture::create_3d_texture_empty(glm::ivec3 dim, GLenum format, GLenum pixel_format, GLenum data_type, GLenum filter, GLenum wrap_mode)
{
	texture t{};
	glAssert(glGenTextures(1, &t.m_handle));
	glAssert(glBindTexture(GL_TEXTURE_3D, t.m_handle));

	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 3));

	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	GLfloat clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glAssert(glTexImage3D(GL_TEXTURE_3D, 0, pixel_format, dim.x, dim.y, dim.z, 0, format, data_type, NULL));
	glAssert(glClearTexImage(GL_TEXTURE_3D, 0, pixel_format, GL_FLOAT, &clear[0]));
	glGenerateTextureMipmap(t.m_handle);
	return t;
}
