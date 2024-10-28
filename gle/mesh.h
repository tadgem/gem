#pragma once

#include "vertex.h"
#include "shape.h"
#include "ecs_system.h"

struct mesh
{
	VAO				m_vao;
	uint32_t		m_index_count;
	aabb			m_original_aabb;
	aabb			m_transformed_aabb;
	uint32_t		m_material_index;

	void draw()
	{
		m_vao.use();
		glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, 0);
	}
};


class mesh_sys : public ecs_system
{
public:

	void			init() override;
	void			update(scene& current_scene) override;
	void			cleanup() override;
	nlohmann::json	serialize(scene& current_scene) override;
	void			deserialize(scene& current_scene, nlohmann::json& sys_json) override;

	~mesh_sys() {}
};