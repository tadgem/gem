#include "gem/mesh.h"
#include "gem/scene.h"
#include "gem/engine.h"

void mesh_sys::init()
{
}

void mesh_sys::cleanup()
{
}

void try_update_mesh_component(mesh_component& mc)
{
	if (engine::assets.get_asset_load_progress(mc.m_handle) == asset_load_progress::loaded)
	{
		auto model_asset = engine::assets.get_asset<model, asset_type::model>(mc.m_handle);
		mc.m_mesh = model_asset->m_data.m_meshes[mc.m_mesh_index];
	}
}

void mesh_sys::update(scene& current_scene)
{
	auto mesh_view = current_scene.m_registry.view<mesh_component>();

	for (auto& [e, meshc] : mesh_view.each())
	{
		if (meshc.m_mesh.m_vao.m_vao_id == INVALID_GL_HANDLE)
		{
			try_update_mesh_component(meshc);
		}
	}
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
		sys_json[get_entity_string(e)] = comp_json;

	}
	return sys_json;
}

void mesh_sys::deserialize(scene& current_scene, nlohmann::json& sys_json)
{
	for (auto [entity, entry] : sys_json.items())
	{
		entt::entity e = get_entity_from_string(entity);
		mesh_component mc{};

		mc.m_handle		= entry["asset_handle"];
		mc.m_mesh_index = entry["mesh_index"];

		try_update_mesh_component(mc);
		
		e = current_scene.m_registry.create(e);
		current_scene.m_registry.emplace<mesh_component>(e, mc);
	}
}

