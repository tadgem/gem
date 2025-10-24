#pragma once
#include "gem/ecs_system.h"
#include "glm.hpp"
#include "gtc/quaternion.hpp"

namespace gem {
class Scene;

struct Transform {
  glm::vec3 m_position{0.0, 0.0, 0.0};
  glm::vec3 m_euler{0.0, 0.0, 0.0};
  glm::vec3 m_scale{1.0, 1.0, 1.0};
  glm::quat m_rotation = glm::quat();

  glm::mat4 m_model = glm::mat4(1.0);
  glm::mat4 m_last_model = glm::mat4(1.0);

  glm::mat3 m_normal_matrix;

  static void UpdateTransforms(Scene &current_scene);
};

class TransformSystem : public ECSSystem {
public:
  TransformSystem() : ECSSystem(HashUtils::GetTypeHash<TransformSystem>()) {}

  void Init() override;
  void Update(Scene &current_scene) override;
  void Cleanup() override;
  nlohmann::json Serialize(Scene &current_scene) override;
  void Deserialize(Scene &current_scene, nlohmann::json &sys_json) override;

  ~TransformSystem() {}
};
} // namespace gem