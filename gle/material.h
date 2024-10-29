#pragma once
#include <string>
#include <any>
#include <map>
#include "shader.h"
#include "texture.h"
#include "model.h"
#include "ecs_system.h"
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

class material_sys : public ecs_system
{
public:

	material_sys() : ecs_system(hash_utils::get_type_hash<material_sys>()) {}
	void			init() override;
	void			update(scene& current_scene) override;
	void			cleanup() override;
	nlohmann::json	serialize(scene& current_scene) override;
	void			deserialize(scene& current_scene, nlohmann::json& sys_json) override;

	~material_sys() {}
};