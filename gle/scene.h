#pragma once
#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <map>
#include "alias.h"
#include "texture.h"
#include "entt.hpp"
class entity;
class model;
class shader;

class scene
{
public:

    scene(const std::string& scene_name);

    entity					create_entity(const std::string& name);
	std::vector<entity>		create_entity_from_model(model& model_to_load, shader& material_shader, glm::vec3 scale = glm::vec3(1.0f), std::map<std::string, texture_map_type> known_maps = {});

	void					on_update();

    const std::string	m_name;
    entt::registry		m_registry;
protected:
	u32					p_created_entity_count;
};

class entity
{
public:
	entt::entity	m_handle;
	scene *    m_scene;

	entity(scene* escene, entt::entity e);

	template<typename _Ty, typename ... Args>
	_Ty&				add_component(Args&& ... args)
	{
		return m_scene->m_registry.emplace<_Ty>(m_handle, std::forward<Args>(args)...);
	}

	template<typename _Ty>
	_Ty&				get_component()
	{
		return m_scene->m_registry.get<_Ty>(m_handle);
	}

	template<typename _Ty>
	std::optional<_Ty>	try_get_component()
	{
		if (!m_scene->m_registry.any_of<_Ty>(m_handle))
		{
			return {};
		}
		return get_component<_Ty>();
	}

	template<typename _Ty>
	bool				has_component()
	{
		if (!m_scene->m_registry.any_of<_Ty>(m_handle))
		{
			return false;
		}
		return true;
	}
};

// component which all entities must have
struct entity_data
{
	std::string m_name;
};