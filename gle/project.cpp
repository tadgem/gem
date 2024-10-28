#include "project.h"
#include "asset_manager.h"
#include "tracy/Tracy.hpp"

nlohmann::json project::serialize(asset_manager& am)
{
	ZoneScoped;
	nlohmann::json json{};

	json["project_name"] = m_name;
	json["scenes"] = m_scene_paths;

	return json;
}

void project::deserialize(asset_manager& am, nlohmann::json& proj_json)
{
	ZoneScoped;
	m_name			= proj_json["project_name"];
	m_scene_paths	= proj_json["scenes"];
}
