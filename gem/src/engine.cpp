#define GLM_ENABLE_EXPERIMENTAL
#include "gem/engine.h"
#include "gem/material.h"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/transform.h"
#include "gem/utils.h"
#include "gem/gl/gl_backend.h"

namespace gem {

void engine::init() {
  ZoneScoped;
  glm::ivec2 resolution = {1920, 1080};
  gpu_backend::init_backend<gl_backend>(backend_init{ resolution, true });

  systems.add_system<transform_sys>();
  systems.add_system<mesh_sys>();
  systems.add_system<material_sys>();
}

void engine::save_project_to_disk(const std::string &filename,
                                  const std::string &directory) {
  ZoneScoped;
  std::string final_path = directory + "/" + filename;
  utils::save_string_to_path(final_path,
                             active_project.serialize(engine::assets).dump());
}

void engine::load_project_from_disk(const std::string &filepath) {
  ZoneScoped;
  nlohmann::json proj_json =
      nlohmann::json::parse(utils::load_string_from_path(filepath));
  project new_proj{};
  new_proj.deserialize(assets, proj_json);
  engine::active_project = new_proj;
}

void engine::shutdown() {
  ZoneScoped;
  systems.m_systems.clear();
  systems.m_system_type_aliases.clear();
}
void engine::update() {
  assets.update();

  for(auto& cb : debug_callbacks.m_callbacks)
  {
    cb();
  }
  debug_callbacks.m_callbacks.clear();
}
} // namespace gem
void debug_callbacks_collection::add(std::function<void()> debug_callback) {
  m_callbacks.emplace_back(debug_callback);
}
