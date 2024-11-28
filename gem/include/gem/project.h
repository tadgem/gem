#pragma once
#include "gem/asset.h"
#include "json.hpp"
#include <string>
#include <vector>
namespace gem {

class asset_manager;
class project {
public:
  std::vector<std::string> m_scene_paths;
  std::vector<serializable_asset_handle> m_project_assets;
  std::string m_name;

  nlohmann::json serialize(asset_manager &am);
  void deserialize(asset_manager &am, nlohmann::json &proj_json);

  GEM_IMPL_ALLOC(project)
};
} // namespace gem