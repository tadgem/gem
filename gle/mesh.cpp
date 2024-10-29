#include "mesh.h"
#include "scene.h"


void mesh_sys::init()
{
}

void mesh_sys::cleanup()
{
}

void mesh_sys::update(scene& current_scene)
{
}

nlohmann::json mesh_sys::serialize(scene& current_scene)
{
	nlohmann::json sys_json{};
	auto view = current_scene.m_registry.view<mesh_component>();
	for (auto [e, mesh] : view.each())
	{
		nlohmann::json comp_json{};
		comp_json["asset_handle"]	= mesh.m_handle;
		comp_json["mesh_index"]		= mesh.m_mesh_index;
		sys_json[static_cast<u32>(e)] = comp_json;

	}
	return sys_json;
}

void mesh_sys::deserialize(scene& current_scene, nlohmann::json& sys_json)
{
	return;
}

