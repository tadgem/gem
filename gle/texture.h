#pragma once
#include <string>
#include "vertex.h"
#include "glm.hpp"
#include "dbg_memory.h"
enum class texture_map_type
{
	diffuse,
	normal,
	specular,
	metallicness,
	roughness,
	ao

};

class texture
{
public:

	texture();
	texture(const std::string& path);
	texture(const std::string& path, std::vector<unsigned char> data);
	~texture();

	void		bind_sampler(GLenum texture_slot, GLenum texture_target = GL_TEXTURE_2D);
	static void bind_sampler_handle(gl_handle handle, GLenum texture_slot, GLenum texture_target = GL_TEXTURE_2D);
	static void bind_image_handle(gl_handle handle, uint32_t binding, uint32_t mip_level, GLenum format);
	static void unbind_image(uint32_t binding);

	int m_width, m_height, m_depth, m_num_channels;

	gl_handle m_handle;
	
	static texture from_data(unsigned int* data, unsigned int count, int width, int height, int depth, int nr_channels);
	static texture create_3d_texture(glm::ivec3 dim, GLenum format, GLenum pixel_format, GLenum data_type, void* data, GLenum filter = GL_LINEAR, GLenum wrap_mode = GL_REPEAT);
	static texture create_3d_texture_empty(glm::ivec3 dim, GLenum format, GLenum pixel_format, GLenum data_type, GLenum filter = GL_LINEAR, GLenum wrap_mode = GL_REPEAT);

	void	load_texture_stbi(std::vector<unsigned char>& data);
	void	load_texture_gli(std::vector<unsigned char>& data);

    void    release();

	inline static texture* white;
	inline static texture* black;

	DEBUG_IMPL_ALLOC(texture);
};

struct sampler_info
{
	GLenum      sampler_slot;
	GLenum      texture_target;
	gl_handle   texture_handle;
};