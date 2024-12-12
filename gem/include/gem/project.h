#pragma once
#include "gem/asset.h"
#include "json.hpp"
#include <string>
#include <vector>
namespace gem {

class AssetManager;
class Project {
public:
  std::vector<std::string> m_scene_paths;
  std::vector<SerializableAssetHandle> m_project_assets;
  std::string m_name;

  nlohmann::json serialize(AssetManager &am);
  void deserialize(AssetManager &am, nlohmann::json &proj_json);

  GEM_IMPL_ALLOC(Project)
};
} // namespace gem