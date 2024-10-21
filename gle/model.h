#pragma once
#include <string>
#include <unordered_map>
#include "mesh.h"
#include "texture.h"
#include "asset.h"
#include "dbg_memory.h"

class model
{
public:

	struct mesh_entry
	{
		std::vector<glm::vec3>	m_positions;
		std::vector<glm::vec3>	m_normals;
		std::vector<glm::vec2>	m_uvs;
		std::vector<u32>		m_indices;
		aabb					m_mesh_aabb;
		u32						m_material_index;
		
		DEBUG_IMPL_ALLOC(mesh_entry)
	};

	struct texture_entry
	{
		texture_map_type	m_map_type;
		asset_handle		m_handle;
		std::string			m_path;
		texture* m_texture;
		DEBUG_IMPL_ALLOC(texture_entry);

	};

	struct material_entry
	{
		std::unordered_map<texture_map_type, texture*> m_material_maps;
		DEBUG_IMPL_ALLOC(material_entry);
	};

	std::vector<mesh>				m_meshes;
	std::vector<material_entry>		        m_materials;
        aabb						m_aabb;

        void release();

	static model load_model_and_textures_from_path(const std::string& path);
	static model load_model_from_path_entries(const std::string& path, std::vector<texture_entry>& texture_entries, std::vector<mesh_entry>& mesh_entries);

	DEBUG_IMPL_ALLOC(model);
};

using model_asset = asset_t<model, asset_type::model>;