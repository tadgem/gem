#pragma once

#include "gem/aabb.h"
#include "gem/asset.h"
#include "gem/dbg_memory.h"
#include "gem/ecs_system.h"
#include "gem/vertex.h"

namespace gem {

struct mesh {
  VAO m_vao;
  uint32_t m_index_count;
  aabb m_original_aabb;
  aabb m_transformed_aabb;
  uint32_t m_material_index;

  void draw() { m_vao.draw(); }
};

struct mesh_component {
  mesh m_mesh;
  asset_handle m_handle;
  u32 m_mesh_index;
};

class mesh_sys : public ecs_system {
public:
  mesh_sys() : ecs_system(hash_utils::get_type_hash<mesh_sys>()) {}

  void init() override;
  void update(scene &current_scene) override;
  void cleanup() override;
  nlohmann::json serialize(scene &current_scene) override;
  void deserialize(scene &current_scene, nlohmann::json &sys_json) override;

  ~mesh_sys() {}
};
} // namespace gem