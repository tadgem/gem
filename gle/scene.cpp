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
	return entity(this, e);
}

std::vector<entity> scene::create_entity_from_model(model& model_to_load, shader& material_shader, glm::vec3 scale, glm::vec3 euler, std::map<std::string, texture_map_type> known_maps)
{
	std::vector<entity> entities{};

	for (auto& entry : model_to_load.m_meshes)
	{
		std::stringstream entity_name;
		entity_name << "Entity " << p_created_entity_count;
		entity e = create_entity(entity_name.str());
		
		transform& trans = e.add_component<transform>();
		trans.m_scale = scale;
		trans.m_euler = euler;
		e.add_component<mesh>(entry);
		material& current_mat = e.add_component<material>(material_shader);

		GLenum texture_slot = GL_TEXTURE0;
		// go through each known map type
		for (auto& [uniform_name, map_type] : known_maps)
		{
			// check if material (instance of shader) has slot for this map type
			
			// check if material has desired map type
			model::material_entry& material_entry = model_to_load.m_materials[entry.m_material_index];
			if (material_entry.m_material_maps.find(map_type) != material_entry.m_material_maps.end())
			{
				current_mat.set_sampler(uniform_name, texture_slot, *material_entry.m_material_maps[map_type], GL_TEXTURE_2D);
				texture_slot++;
			}
		}

		entities.push_back(e);
	}

	return entities;
}

bool scene::does_entity_exist(u32 index)
{
	return m_registry.valid(entt::entity {index});
}

void scene::on_update()
{
	transform::update_transforms(*this);
}

entity::entity(scene* escene, entt::entity e) : m_scene(escene), m_handle(e)
{
}
