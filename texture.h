#pragma once
#include <string>

enum class texture_map_type
{
	diffuse,
	normal,
	specular,

};

class texture
{
public:

	texture(const std::string& path);

	int m_width, m_height, m_depth, m_num_channels;

};