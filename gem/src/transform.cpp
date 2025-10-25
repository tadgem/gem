#define GLM_ENABLE_EXPERIMENTAL

#include "gem/transform.h"
#include "gem/json_type_interop.hpp"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/scene.h"
#include "gem/utils.h"

namespace gem {

void Transform::UpdateTransforms(Scene &current_scene) {
  ZoneScoped;
  auto transform_view = current_scene.registry.view<Transform>();

  for (auto [e, trans] : transform_view.each()) {
    trans.m_last_model = trans.m_model;
    trans.m_model =
        Utils::GetModelMatrix(trans.m_position, trans.m_euler, trans.m_scale);

    trans.m_normal_matrix = Utils::GetNormalMatrix(trans.m_model);
  }

  auto transform_mesh_view = current_scene.registry.view<Transform, MeshComponent>();
  for (auto [e, trans, mesh] : transform_mesh_view.each()) {
    mesh.mesh->transformed_aabb =
        Utils::TransformAABB(mesh.mesh->original_aabb, trans.m_model);
  }
}

void TransformSystem::Init() { ZoneScoped; }

void TransformSystem::Cleanup() { ZoneScoped; }

void TransformSystem::Update(Scene &current_scene) {
  ZoneScoped;
  Transform::UpdateTransforms(current_scene);
}

nlohmann::json TransformSystem::Serialize(Scene &current_scene) {
  ZoneScoped;
  nlohmann::json sys_json;

  auto sys_view = current_scene.registry.view<Transform>();

  for (auto [e, transform] : sys_view.each()) {
    nlohmann::json comp_json;
    comp_json["position"] = transform.m_position;
    comp_json["euler"] = transform.m_euler;
    comp_json["scale"] = transform.m_scale;
    sys_json[GetEntityIDString(e)] = comp_json;
  }

  return sys_json;
}

void TransformSystem::Deserialize(Scene &current_scene,
                                nlohmann::json &sys_json) {
  ZoneScoped;
  for (auto [entity, entry] : sys_json.items()) {
    entt::entity e = GetEntityIDFromString(entity);
    Transform t{};
    t.m_position = entry["position"];
    t.m_euler = entry["euler"];
    t.m_scale = entry["scale"];

    e = current_scene.registry.create(e);
    current_scene.registry.emplace<Transform>(e, t);
  }
}
} // namespace gem