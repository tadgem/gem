#include "transform.h"
#include "scene.h"
#include "utils.h"
#include "mesh.h"

void transform::update_transforms(scene& current_scene)
{
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
