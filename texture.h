#pragma once
#include <string>

class texture
{
public:

	texture(const std::string& path);

	int m_width, m_height, m_depth, m_num_channels;

};