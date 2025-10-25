#include "gem/project.h"
#include "gem/asset_manager.h"
#include "gem/profile.h"

namespace gem {

nlohmann::json Project::Serialize(AssetManager &am) {
  ZoneScoped;
  nlohmann::json json{};

  json["project_name"] = project_name;
  json["project_assets"] = project_assets;
  json["scenes"] = scene_paths;

  return json;
}

void Project::Deserialize(AssetManager &am, nlohmann::json &proj_json) {
  ZoneScoped;
  project_name = proj_json["project_name"];
  project_assets = proj_json["project_assets"];
  scene_paths = proj_json["scenes"];
}
} // namespace gem