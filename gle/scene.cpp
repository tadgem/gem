#include "scene.h"
#include "model.h"
#include "shader.h"
#include "transform.h"
#include "material.h"
#include "utils.h"
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

std::vector<entity> scene::create_entity_from_model(
	model& model_to_load, 
	shader& material_shader, 
	glm::vec3 scale, 
	glm::vec3 euler, 
	std::map<std::string, texture_map_type> known_maps)
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
			// check if material has desired map type
			model::material_entry& material_entry = model_to_load.m_materials[entry.m_material_index];
			if (material_entry.m_material_maps.find(map_type) != material_entry.m_material_maps.end())
			{
				current_mat.set_sampler(
						uniform_name, 
						texture_slot, 
						material_entry.m_material_maps[map_type], 
						GL_TEXTURE_2D);
				
				texture_slot++;
			}
		}
		entities.push_back(e);
	}

	// TODO: Update to work from scene overall aabb
	m_scene_bounding_volume = utils::transform_aabb(model_to_load.m_aabb, utils::get_model_matrix(glm::vec3(0.0), euler, scale));

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

void scene::update_aabb(aabb& in)
{
	if (in.min.x < m_scene_bounding_volume.min.x)
	{
		m_scene_bounding_volume.min.x = in.min.x;
	}
	if (in.min.y < m_scene_bounding_volume.min.y)
	{
		m_scene_bounding_volume.min.y = in.min.y;
	}
	if (in.min.z < m_scene_bounding_volume.min.z)
	{
		m_scene_bounding_volume.min.z = in.min.z;
	}

	if (in.max.x > m_scene_bounding_volume.max.x)
	{
		m_scene_bounding_volume.max.x = in.max.x;
	}
	if (in.max.y > m_scene_bounding_volume.max.y)
	{
		m_scene_bounding_volume.max.y = in.max.y;
	}
	if (in.max.z > m_scene_bounding_volume.max.z)
	{
		m_scene_bounding_volume.max.z = in.max.z;
	}
}

entity::entity(scene* escene, entt::entity e) : m_scene(escene), m_handle(e)
{
}


scene_manager::scene_manager()
{

}

void scene_manager::close_scene(hash_string scene_hash)
{

}

scene* scene_manager::get_scene(hash_string scene_hash)
{
	if (p_active_scenes.find(scene_hash) == p_active_scenes.end())
	{
		return nullptr;
	}
	return p_active_scenes[scene_hash].get();
}

scene* scene_manager::create_scene(const std::string& scene_name)
{
	hash_string scene_hash(scene_name);
	if (p_active_scenes.find(scene_hash) != p_active_scenes.end())
	{
		return p_active_scenes[scene_hash].get();
	}

	p_active_scenes.emplace(scene_hash, std::make_unique<scene>(scene_name));
	return p_active_scenes[scene_hash].get();
}

scene* scene_manager::load_scene(nlohmann::json& scene_json)
{
	return nullptr;
}