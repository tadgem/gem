#pragma once
#include <string>
#include <any>
#include <map>
#include "shader.h"
#include "texture.h"
#include "model.h"

#define ENABLE_MATERIAL_UNIFORM_CHECKS

class scene;
class asset_manager;

class material
{
public:
	material(shader& shader_program);

	std::map<std::string, shader::uniform_type>	m_uniforms;
	std::map<std::string, std::any>				m_uniform_values;

	template<typename _Ty>
	bool	set_uniform_value(const std::string& name, const _Ty& val)
	{
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
		if (m_uniforms.find(name) == m_uniforms.end())
		{
			return false;
		}
#endif
		m_uniform_values[name] = val;
		return true;
	}

	bool    set_sampler(const std::string& sampler_name, GLenum texture_slot, texture_entry &tex_entry, GLenum texture_target = GL_TEXTURE_2D);

	void	bind_material_uniforms(asset_manager& am);

	shader& m_prog;

};