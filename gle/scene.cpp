#include "scene.h"

scene::scene(const std::string& scene_name) : m_name(scene_name)
{
}

entity scene::create_entity(const std::string& name)
{
	entt::entity e = m_registry.create();
	m_registry.emplace<entity_data>(e, entity_data{ name });
	return entity{ e, this };
}
