#pragma once
#include <vector>
#include <string>
#include "asset.h"
#include "serialization.h"

class asset_manager;
class project
{
public:
	std::vector<std::string>					m_scene_paths;
	std::vector<serializable_asset_handle>		m_project_assets;
	std::string									m_name;

	template<typename Archive>
	void serialize(Archive& ar) {
		ZoneScoped;
		ar(m_scene_paths, m_project_assets, m_name);
	}
};