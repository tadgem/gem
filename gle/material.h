#pragma once
#include <string>
#include <unordered_map>
#include "shader.h"
class material
{
public:
	material(shader& shader_program);

	std::unordered_map<std::string, shader::uniform_type> m_uniforms;
};