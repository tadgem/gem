#include "gem/asset.h"
#include "gem/hash_string.h"
#include "gem/profile.h"
namespace gem {
Asset::Asset(const std::string &path, AssetType type)
    : m_path(path), m_handle(path, type) {
  ZoneScoped;
}

std::string get_asset_type_name(const AssetType &t) {
  ZoneScoped;
  switch (t) {
  case AssetType::model:
    return "model";
  case AssetType::text:
    return "text";
  case AssetType::texture:
    return "texture";
  case AssetType::shader:
    return "shader";
  }

  return "unknown";
}

AssetHandle::AssetHandle(const std::string &path, AssetType type) {
  ZoneScoped;
  m_type = type;
  m_path_hash = HashUtils::GetStringHash(path);
}

AssetHandle::AssetHandle(const HashString &path_hash, AssetType type) {
  ZoneScoped;
  m_type = type;
  m_path_hash = path_hash;
}

} // namespace gem