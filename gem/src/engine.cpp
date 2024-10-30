#include "gem/engine.h"
#include "gem/utils.h"
#include "gem/transform.h"
#include "gem/mesh.h"
#include "gem/material.h"
#include "gem/profile.h"
namespace gem {

	void engine::init()
	{
		ZoneScoped;
		systems.add_system<transform_sys>();
		systems.add_system<mesh_sys>();
		systems.add_system<material_sys>();
	}

	void engine::save_project_to_disk(const std::string& filename, const std::string& directory)
	{
		ZoneScoped;
		std::string final_path = directory + "/" + filename;
		utils::save_string_to_path(final_path, active_project.serialize(engine::assets).dump());
	}

	void engine::load_project_from_disk(const std::string& filepath)
	{
		ZoneScoped;
		nlohmann::json proj_json = nlohmann::json::parse(utils::load_string_from_path(filepath));
		project new_proj{};
		new_proj.deserialize(assets, proj_json);
		engine::active_project = new_proj;
	}

	void engine::shutdown()
	{
		ZoneScoped;
		systems.m_systems.clear();
		systems.m_system_type_aliases.clear();
	}
}