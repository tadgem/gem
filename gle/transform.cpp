#include "transform.h"
#include "scene.h"
#include "utils.h"
#include "mesh.h"
#include "profile.h"
#include "serialization.h"
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
}

void transform_sys::cleanup()
{
}

void transform_sys::update(scene& current_scene)
{
	transform::update_transforms(current_scene);
}

//nlohmann::json transform_sys::serialize(scene& current_scene)
//{
//	nlohmann::json sys_json;
//
//	auto sys_view = current_scene.m_registry.view<transform>();
//
//	for (auto [e, transform] : sys_view.each())
//	{
//		nlohmann::json comp_json;
//		comp_json["position"] = transform.m_position;
//		comp_json["euler"] = transform.m_euler;
//		comp_json["scale"] = transform.m_scale;
//		sys_json[static_cast<u32>(e)] = comp_json;
//	}
//
//	return sys_json;
//}
//
//void transform_sys::deserialize(scene& current_scene, nlohmann::json& sys_json)
//{
//	return;
//}

