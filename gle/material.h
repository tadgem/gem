#pragma once
#include <string>
#include <any>
#include <unordered_map>
#include "shader.h"
#include "texture.h"

#define ENABLE_MATERIAL_UNIFORM_CHECKS

class material
{
public:
	material(shader& shader_program);

	std::unordered_map<std::string, shader::uniform_type>	m_uniforms;

	template<typename _Ty>
	bool	set_uniform_value(const std::string& name, const _Ty& val)
	{
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
		if (m_uniforms.find(name) == m_uniforms.end())
		{
			return false;
		}
#endif
		p_uniform_values[name] = val;
		return true;
	}

	bool    set_sampler(const std::string& sampler_name, GLenum texture_slot, texture& tex, GLenum texture_target = GL_TEXTURE_2D);

	void	bind_material_uniforms(shader& prog);

private:
	std::unordered_map<std::string, std::any>				p_uniform_values;

};