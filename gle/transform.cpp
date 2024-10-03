#include "transform.h"
#include "scene.h"
#include "utils.h"


void transform::update_transforms(scene& current_scene)
{
	auto& transform_view = current_scene.m_registry.view<transform>();

	for (auto& [e, trans] : transform_view.each())
	{
		trans.m_last_model = trans.m_model;
		trans.m_model = utils::get_model_matrix(trans.m_position, trans.m_euler, trans.m_scale);
	}
}
