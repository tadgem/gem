#include "texture.h"
#include <iostream>
#include "GL/glew.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

texture::texture(const std::string& path)
{
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(path.c_str(), & m_width, & m_height, &m_num_channels, 0);

	if (!data)
	{
		std::cerr << "Failed to load texture at path : " << path << std::endl;
		return;
	}

	glGenTextures(1, &m_handle);
	glBindTexture(GL_TEXTURE_2D, m_handle);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
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
	glGenTextures(1, &t.m_handle);
	glBindTexture(GL_TEXTURE_3D, t.m_handle);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
	glTexImage3D(GL_TEXTURE_3D, 0, pixel_format, dim.x, dim.y, dim.z, 0, format, data_type, data);
	return t;
}
