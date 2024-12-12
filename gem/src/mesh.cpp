#define GLM_ENABLE_EXPERIMENTAL

#include "gem/mesh.h"
#include "gem/engine.h"
#include "gem/scene.h"

namespace gem {

void MeshSystem::init() { ZoneScoped; }

void MeshSystem::cleanup() { ZoneScoped; }

void try_update_mesh_component(MeshComponent &mc) {
  ZoneScoped;

  if (Engine::assets.get_asset_load_progress(mc.m_handle) ==
      AssetLoadProgress::loaded) {
    auto model_asset =
        Engine::assets.get_asset<Model, AssetType::model>(mc.m_handle);
    mc.m_mesh = model_asset->m_data.m_meshes[mc.m_mesh_index];
  }
}

void MeshSystem::update(Scene &current_scene) {
  ZoneScoped;

  auto mesh_view = current_scene.m_registry.view<MeshComponent>();

  for (auto &[e, meshc] : mesh_view.each()) {
    if (meshc.m_mesh.m_vao.m_vao_id == INVALID_GL_HANDLE) {
      try_update_mesh_component(meshc);
    }
  }
}

nlohmann::json MeshSystem::serialize(Scene &current_scene) {
  ZoneScoped;

  nlohmann::json sys_json{};
  auto view = current_scene.m_registry.view<MeshComponent>();
  for (auto [e, mesh] : view.each()) {
    nlohmann::json comp_json{};
    comp_json["asset_handle"] = mesh.m_handle;
    comp_json["mesh_index"] = mesh.m_mesh_index;
    sys_json[get_entity_string(e)] = comp_json;
  }
  return sys_json;
}

void MeshSystem::deserialize(Scene &current_scene, nlohmann::json &sys_json) {
  ZoneScoped;

  for (auto [entity, entry] : sys_json.items()) {
    entt::entity e = get_entity_from_string(entity);
    MeshComponent mc{};

    mc.m_handle = entry["asset_handle"];
    mc.m_mesh_index = entry["mesh_index"];

    // todo: only do this on update, allow scene serialization to be async
    // might be ok but could hit race
    try_update_mesh_component(mc);

    e = current_scene.m_registry.create(e);
    current_scene.m_registry.emplace<MeshComponent>(e, mc);
  }
}

} // namespace gem