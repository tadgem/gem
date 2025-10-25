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
  uint32_t  index_count;
  AABB      original_aabb;
  AABB      transformed_aabb;
  uint32_t  material_index;
  VAO       vao;
};

struct MeshComponent {
  AMesh* mesh;
  AssetHandle handle;
  u32 mesh_index = 0;
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