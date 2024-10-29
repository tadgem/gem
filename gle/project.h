#pragma once
#include <vector>
#include <string>
#include "asset.h"
#include "json.hpp"

class asset_manager;
class project
{
public:
	std::vector<std::string>					m_scene_paths;
	std::vector<serializable_asset_handle>		m_project_assets;
	std::string									m_name;

	nlohmann::json				serialize(asset_manager& am);
	void						deserialize(asset_manager& am, nlohmann::json& proj_json);
};