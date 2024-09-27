#pragma once
#include <string>
#include <optional>
#include <memory>
#include "entt.hpp"
class scene;

class entity
{
public:
	entt::entity	m_handle;
	scene*			m_scene;

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

class scene
{
public:

	scene(const std::string& scene_name);

	entity		create_entity(const std::string& name);

	const std::string	m_name;
	entt::registry		m_registry;
};