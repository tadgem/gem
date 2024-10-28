#include "engine.h"
#include "utils.h"

void engine::save_project_to_disk(const std::string& filename, const std::string& directory)
{
	std::string final_path = directory + "/" + filename;
	utils::save_string_to_path(final_path, active_project.serialize(engine::assets).dump());
}

void engine::load_project_from_disk(const std::string& filepath)
{
	nlohmann::json proj_json = nlohmann::json::parse(utils::load_string_from_path(filepath));
	project new_proj{};
	new_proj.deserialize(assets, proj_json);
	engine::active_project = new_proj;
}
