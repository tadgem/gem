#pragma once

#include "ecs_system.h"
#include "glm.hpp"
#include "gtc/quaternion.hpp"

class scene;

struct transform
{
	glm::vec3 m_position	{ 0.0, 0.0, 0.0 };
	glm::vec3 m_euler		{ 0.0, 0.0, 0.0 };
	glm::vec3 m_scale		{ 1.0, 1.0, 1.0 };
	glm::quat m_rotation	= glm::quat();

	glm::mat4 m_model		= glm::mat4(1.0);
	glm::mat4 m_last_model	= glm::mat4(1.0);

	glm::mat3 m_normal_matrix;

	static void update_transforms(scene& current_scene);
};


class transform_sys : public ecs_system
{
public:

	transform_sys() : ecs_system(hash_utils::get_type_hash<transform_sys>()) {}

	void			init() override;
	void			update(scene& current_scene) override;
	void			cleanup() override;/*
	nlohmann::json	serialize(scene& current_scene) override;
	void			deserialize(scene& current_scene, nlohmann::json& sys_json) override;*/

	~transform_sys() {}

};