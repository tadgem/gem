#include "gem/transform.h"
#include "gem/scene.h"
#include "gem/utils.h"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/json_type_interop.hpp"

void transform::update_transforms(scene& current_scene)
{
	ZoneScoped;
	auto transform_view = current_scene.m_registry.view<transform>();

	for (auto [e, trans] : transform_view.each())
	{
		trans.m_last_model = trans.m_model;
		trans.m_model = utils::get_model_matrix(trans.m_position, trans.m_euler, trans.m_scale);
		trans.m_normal_matrix = utils::get_normal_matrix(trans.m_model);
	}

	auto transform_mesh_view = current_scene.m_registry.view<transform, mesh>();
	for (auto [e, trans, mesh] : transform_mesh_view.each())
	{
		mesh.m_transformed_aabb = utils::transform_aabb(mesh.m_original_aabb, trans.m_model);
	}
}

void transform_sys::init()
{
	ZoneScoped;

}

void transform_sys::cleanup()
{
	ZoneScoped;

}

void transform_sys::update(scene& current_scene)
{
	ZoneScoped;
	transform::update_transforms(current_scene);
}

nlohmann::json transform_sys::serialize(scene& current_scene)
{
	ZoneScoped;
	nlohmann::json sys_json;

	auto sys_view = current_scene.m_registry.view<transform>();

	for (auto [e, transform] : sys_view.each())
	{
		nlohmann::json comp_json;
		comp_json["position"] = transform.m_position;
		comp_json["euler"] = transform.m_euler;
		comp_json["scale"] = transform.m_scale;
		sys_json[get_entity_string(e)] = comp_json;
	}

	return sys_json;
}

void transform_sys::deserialize(scene& current_scene, nlohmann::json& sys_json)
{
	ZoneScoped;
	for (auto [entity,  entry ] : sys_json.items())
	{
		entt::entity e = get_entity_from_string(entity);
		transform t{};
		t.m_position = entry["position"];
		t.m_euler = entry["euler"];
		t.m_scale = entry["scale"];

		e = current_scene.m_registry.create(e);
		current_scene.m_registry.emplace<transform>(e, t);
	}
}

