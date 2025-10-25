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

  systems.AddSystem<TransformSystem>();
  systems.AddSystem<MeshSystem>();
  systems.AddSystem<MaterialSystem>();
}

void Engine::SaveProjectToDisk(const std::string &filename,
                                  const std::string &directory) {
  ZoneScoped;
  std::string final_path = directory + "/" + filename;
  Utils::SaveStringToPath(final_path,
                             active_project.Serialize(Engine::assets).dump());
}

void Engine::LoadProjectFromDisk(const std::string &filepath) {
  ZoneScoped;
  nlohmann::json proj_json =
      nlohmann::json::parse(Utils::LoadStringFromPath(filepath));
  Project new_proj{};
  new_proj.Deserialize(assets, proj_json);
  Engine::active_project = new_proj;
}

void Engine::Shutdown() {
  ZoneScoped;
  systems.systems.clear();
  systems.system_type_aliases.clear();
}
void Engine::Update() {
  assets.Update();

  for (auto &cb : debug_callbacks.callbacks) {
    cb();
  }
  debug_callbacks.callbacks.clear();
}
} // namespace gem
void DebugCallbackCollection::Add(std::function<void()> debug_callback) {
  callbacks.emplace_back(debug_callback);
}
