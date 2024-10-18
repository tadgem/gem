#pragma once
#include <string>
#include <unordered_map>
#include "mesh.h"
#include "texture.h"
#include "asset.h"

class model
{
public:

	struct mesh_entry
	{
		std::vector<glm::vec3>	m_positions;
		std::vector<glm::vec3>	m_normals;
		std::vector<glm::vec2>	m_uvs;
		std::vector<u32>		m_indices;
	};

	struct texture_entry
	{
		texture_map_type	m_map_type;
		asset_handle		m_handle;
		std::string			m_path;
		texture* m_texture;
	};

	struct material_entry
	{
		std::unordered_map<texture_map_type, texture*> m_material_maps;
	};

	std::vector<mesh>				m_meshes;
	std::vector<material_entry>		m_materials;
	std::vector<mesh_entry>			m_mesh_entries;
	aabb							m_aabb;

	static model load_model_and_textures_from_path(const std::string& path);
	static model load_model_from_path_entries(const std::string& path, std::vector<texture_entry>& texture_entries);
};

using model_asset = asset_t<model, asset_type::model>;