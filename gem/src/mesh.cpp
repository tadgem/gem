#define GLM_ENABLE_EXPERIMENTAL

#include "gem/mesh.h"
#include "gem/engine.h"
#include "gem/scene.h"

namespace gem {

void MeshSystem::Init() { ZoneScoped; }

void MeshSystem::Cleanup() { ZoneScoped; }

void try_update_mesh_component(MeshComponent &mc) {
  ZoneScoped;

  if (Engine::assets.GetLoadProgress(mc.handle) ==
      AssetLoadProgress::loaded) {
    auto model_asset =
        Engine::assets.GetAsset<Model, AssetType::kModel>(mc.handle);
    mc.mesh = model_asset->data.meshes[mc.mesh_index];
  }
}

void MeshSystem::Update(Scene &current_scene) {
  ZoneScoped;

  auto mesh_view = current_scene.registry.view<MeshComponent>();

  for (auto [e, meshc] : mesh_view.each()) {
    if (meshc.mesh->vao.vao_handle == INVALID_GL_HANDLE) {
      try_update_mesh_component(meshc);
    }
  }
}

nlohmann::json MeshSystem::Serialize(Scene &current_scene) {
  ZoneScoped;

  nlohmann::json sys_json{};
  auto view = current_scene.registry.view<MeshComponent>();
  for (auto [e, mesh] : view.each()) {
    nlohmann::json comp_json{};
    comp_json["asset_handle"] = mesh.handle;
    comp_json["mesh_index"] = mesh.mesh_index;
    sys_json[GetEntityIDString(e)] = comp_json;
  }
  return sys_json;
}

void MeshSystem::Deserialize(Scene &current_scene, nlohmann::json &sys_json) {
  ZoneScoped;

  for (auto [entity, entry] : sys_json.items()) {
    entt::entity e = GetEntityIDFromString(entity);
    MeshComponent mc{};

    mc.handle = entry["asset_handle"];
    mc.mesh_index = entry["mesh_index"];

    // todo: only do this on update, allow scene serialization to be async
    // might be ok but could hit race
    try_update_mesh_component(mc);

    e = current_scene.registry.create(e);
    current_scene.registry.emplace<MeshComponent>(e, mc);
  }
}

} // namespace gem