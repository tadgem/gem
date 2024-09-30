#include "texture.h"
#include "texture.h"
#include "texture.h"
#include "texture.h"
#include "texture.h"
#include <iostream>
#include "GL/glew.h"
#include "SOIL/SOIL.h"
#include "gl.h"
#include "stb_image.h"
#include "utils.h"
struct DDS_PIXELFORMAT {
	unsigned long dwSize;
	unsigned long dwFlags;
	union {
		unsigned long		dwFourCC;
		char				dwFourCC_Chars[4];
	};
	unsigned long dwRGBBitCount;
	unsigned long dwRBitMask;
	unsigned long dwGBitMask;
	unsigned long dwBBitMask;
	unsigned long dwABitMask;
};

typedef struct {
	unsigned long			dwMagicNumber;
	unsigned long           dwSize;
	unsigned long           dwFlags;
	unsigned long           dwHeight;
	unsigned long           dwWidth;
	unsigned long           dwPitchOrLinearSize;
	unsigned long           dwDepth;
	unsigned long           dwMipMapCount;
	unsigned long           dwReserved1[11];
	DDS_PIXELFORMAT			ddspf;
	unsigned long           dwCaps;
	unsigned long           dwCaps2;
	unsigned long           dwCaps3;
	unsigned long           dwCaps4;
	unsigned long           dwReserved2;
} DDS_HEADER;

texture::texture(const std::string& path)
{
	//unsigned char* data = stbi_load(path.c_str(), & m_width, & m_height, &m_num_channels, 0);
	std::vector<u8> data_o = utils::load_binary_from_path(path);
	unsigned char* data = SOIL_load_image_from_memory(data_o.data(), data_o.size(),  & m_width, &m_height, &m_num_channels, SOIL_LOAD_AUTO);
	if (!data)
	{
		std::cerr << "Failed to load texture at path : " << path << std::endl;
		return;
	}
	std::string compressed_format_type = "";
	int			block_size = -1;
	if (path.find("dds"))
	{
		DDS_HEADER header{};
		memcpy(&header, &data_o[0], sizeof(DDS_HEADER));
		compressed_format_type = std::string(header.ddspf.dwFourCC_Chars, 4);
		std::cout << "DDS File, compression format: " << compressed_format_type << "\n";
		std::cout << "R Mask" << std::hex << header.ddspf.dwRBitMask << ", ";
		std::cout << "G Mask" << std::hex << header.ddspf.dwGBitMask << ", ";
		std::cout << "B Mask" << std::hex << header.ddspf.dwBBitMask << ", ";
		std::cout << "A Mask" << std::hex << header.ddspf.dwABitMask << "\n";
	}

	glGenTextures(1, &m_handle);
	glBindTexture(GL_TEXTURE_2D, m_handle);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLenum format = GL_RGBA;
	if (m_num_channels < 4)
	{
		format = GL_RGB;
	}
	/*if (compressed_format_type == "DXT1")
	{
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		block_size = 8;
	}
	if (compressed_format_type == "DXT3")
	{
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		block_size = 16;
	}
	if (compressed_format_type == "DXT5")
	{
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		block_size = 16;
	}*/
	if (compressed_format_type == "ATI2")
	{
		format = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
		block_size = 16;
	}
	if (format == GL_RGBA || format == GL_RGB)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		unsigned int size = ((m_width + 3) / 4) * ((m_height + 3) / 4) * block_size;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, size, data);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(data);
}

void texture::bind_sampler(GLenum texture_slot, GLenum texture_target)
{
	bind_sampler_handle(m_handle, texture_slot, texture_target);
}

void texture::bind_sampler_handle(gl_handle handle, GLenum texture_slot, GLenum texture_target)
{
	glAssert(glActiveTexture(texture_slot));
	glAssert(glBindTexture(texture_target, handle));
}

void texture::bind_image_handle(gl_handle handle, uint32_t binding, uint32_t mip_level, GLenum format)
{
	glAssert(glBindImageTexture(binding, handle, mip_level, GL_TRUE, 0, GL_READ_WRITE, format));

}

void texture::unbind_image(uint32_t binding)
{
	glAssert(glBindImageTexture(binding, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8));
}

texture texture::from_data(unsigned int* data, unsigned int count, int width, int height, int depth, int nr_channels)
{
	texture t{};
	glGenTextures(1, &t.m_handle);
	glBindTexture(GL_TEXTURE_2D, t.m_handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);

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
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 5));

	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST));
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
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 5));

	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	GLfloat clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glAssert(glTexImage3D(GL_TEXTURE_3D, 0, pixel_format, dim.x, dim.y, dim.z, 0, format, data_type, NULL));
	glAssert(glClearTexImage(GL_TEXTURE_3D, 0, pixel_format, GL_FLOAT, &clear[0]));
	glGenerateTextureMipmap(t.m_handle);
	return t;
}
