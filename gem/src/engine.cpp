#define GLM_ENABLE_EXPERIMENTAL
#include "gem/engine.h"
#include "gem/gl/gl_backend.h"
#include "gem/material.h"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/transform.h"
#include "gem/utils.h"

namespace gem {

void Engine::Init(const glm::ivec2& resolution) {
  ZoneScoped;
  BackendInit init_props = {resolution, true};
  GPUBackend::InitBackend<GLBackend>(init_props);

  systems.add_system<TransformSystem>();
  systems.add_system<MeshSystem>();
  systems.add_system<MaterialSystem>();
}

void Engine::SaveProjectToDisk(const std::string &filename,
                                  const std::string &directory) {
  ZoneScoped;
  std::string final_path = directory + "/" + filename;
  Utils::save_string_to_path(final_path,
                             active_project.serialize(Engine::assets).dump());
}

void Engine::LoadProjectFromDisk(const std::string &filepath) {
  ZoneScoped;
  nlohmann::json proj_json =
      nlohmann::json::parse(Utils::load_string_from_path(filepath));
  Project new_proj{};
  new_proj.deserialize(assets, proj_json);
  Engine::active_project = new_proj;
}

void Engine::Shutdown() {
  ZoneScoped;
  systems.m_systems.clear();
  systems.m_system_type_aliases.clear();
}
void Engine::Update() {
  assets.update();

  for (auto &cb : debug_callbacks.m_callbacks) {
    cb();
  }
  debug_callbacks.m_callbacks.clear();
}
} // namespace gem
void DebugCallbackCollection::Add(std::function<void()> debug_callback) {
  m_callbacks.emplace_back(debug_callback);
}
