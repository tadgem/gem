#include "project.h"
#include "asset_manager.h"

nlohmann::json project::serialize(asset_manager& am)
{
	return nlohmann::json();
}

void project::deserialize(asset_manager& am, nlohmann::json& proj_json)
{
}
