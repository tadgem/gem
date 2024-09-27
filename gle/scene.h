#pragma once
#include <string>
#include "entt.hpp"

class scene;

class entity
{
public:
	entt::entity	m_handle;
	scene*			m_scene;
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