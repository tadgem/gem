#pragma once
#include <string>
#include <unordered_map>
#include "vertex.h"
#include "shape.h"
#include "texture.h"

struct mesh
{
	VAO				m_vao;
	uint32_t		m_index_count;
	shapes::aabb	m_aabb;
	uint32_t		m_material_index;
};

class model
{
public:

	struct material_entry
	{
		std::unordered_map<texture_map_type, std::string> m_map_paths;
	};

	std::vector<mesh> m_meshes;

	static model load_model_from_path(const std::string& path);
};