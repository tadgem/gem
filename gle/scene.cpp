#include "scene.h"
#include "model.h"
#include "shader.h"
#include "transform.h"
#include "material.h"

#include <sstream>


scene::scene(const std::string& scene_name) : m_name(scene_name)
{
}

entity scene::create_entity(const std::string& name)
{
	entt::entity e = m_registry.create();
	m_registry.emplace<entity_data>(e, entity_data{ name });
	p_created_entity_count++;
	return entity{ e, this };
}

std::vector<entity> scene::create_entity_from_model(model& model_to_load, shader& material_shader)
{
	std::vector<entity> entities{};
	std::stringstream entity_name;

	for (auto& entry : model_to_load.m_meshes)
	{
		entity_name << "Entity " << p_created_entity_count;
		entity e = create_entity(entity_name.str());
		entity_name.clear();

		e.add_component<transform>();
		e.add_component<mesh>(entry);
		e.add_component<material>(material_shader);

		entities.push_back(e);
	}

	return entities;
}

void scene::on_update()
{
	transform::update_transforms(*this);
}

