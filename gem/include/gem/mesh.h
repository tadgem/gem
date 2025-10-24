#pragma once
#include "gem/AABB.h"
#include "gem/asset.h"
#include "gem/dbg_memory.h"
#include "gem/ecs_system.h"
#include "gem/vertex.h"

namespace gem {

class AMesh
{
public:
  uint32_t  m_index_count;
  AABB      m_original_aabb;
  AABB      m_transformed_aabb;
  uint32_t  m_material_index;
  VAO       m_vao;
};

struct MeshComponent {
  AMesh* m_mesh;
  AssetHandle m_handle;
  u32 m_mesh_index = 0;
};

class MeshSystem : public ECSSystem {
public:
  MeshSystem() : ECSSystem(HashUtils::GetTypeHash<MeshSystem>()) {}

  void Init() override;
  void Update(Scene &current_scene) override;
  void Cleanup() override;
  nlohmann::json Serialize(Scene &current_scene) override;
  void Deserialize(Scene &current_scene, nlohmann::json &sys_json) override;

  ~MeshSystem() override {}
};
} // namespace gem