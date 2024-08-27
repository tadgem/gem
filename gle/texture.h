#pragma once
#include <string>
#include "vertex.h"

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

	texture() = default;
	texture(const std::string& path);

	int m_width, m_height, m_depth, m_num_channels;

	gl_handle m_handle;

	static texture from_data(unsigned int* data, unsigned int count, int width, int height, int depth, int nr_channels);

	inline static texture* white;
	inline static texture* black;
};