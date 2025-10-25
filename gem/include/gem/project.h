#pragma once
#include "gem/asset.h"
#include "json.hpp"
#include <string>
#include <vector>
namespace gem {

class AssetManager;
class Project {
public:
  std::vector<std::string> scene_paths;
  std::vector<SerializableAssetHandle> project_assets;
  std::string project_name;

  nlohmann::json Serialize(AssetManager &am);
  void Deserialize(AssetManager &am, nlohmann::json &proj_json);

  GEM_IMPL_ALLOC(Project)
};
} // namespace gem