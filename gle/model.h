#pragma once
#include <string>
#include <unordered_map>
#include "mesh.h"
#include "texture.h"


class model
{
public:
	struct material_entry
	{
		std::unordered_map<texture_map_type, texture> m_material_maps;
	};

	std::vector<mesh>				m_meshes;
	std::vector<material_entry>		m_materials;
	aabb							m_aabb;

	static model load_model_from_path(const std::string& path);
};