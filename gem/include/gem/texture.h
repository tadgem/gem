#pragma once
#include <string>
#include "glm.hpp"
#include "gem/vertex.h"
#include "gem/dbg_memory.h"
#include "gem/asset.h"

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

	gl_handle m_handle = INVALID_GL_HANDLE;
	
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

struct texture_entry
{
	texture_map_type	m_map_type;
	asset_handle		m_handle;
	std::string			m_path;
	texture*			m_texture = nullptr;

	texture_entry() { m_texture = nullptr; };
	texture_entry(texture_map_type tmt, asset_handle ah, const std::string& path, texture* data);
	DEBUG_IMPL_ALLOC(texture_entry);
};

struct sampler_info
{
	GLenum					sampler_slot;
	GLenum					texture_target;
	texture_entry			tex_entry;
};