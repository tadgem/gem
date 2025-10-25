#pragma once
#include "entt/entt.hpp"
#include "gem/dbg_memory.h"
#include "gem/hash_string.h"
#include "json.hpp"
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace gem {
class Scene;
class AssetManager;
class ECSSystem {
public:
  const HashString kSysHash;

  ECSSystem(HashString sys_hash) : kSysHash(sys_hash) {}

  virtual void Init() = 0;
  virtual void Update(Scene &current_scene) = 0;
  virtual void Cleanup() = 0;

  virtual nlohmann::json Serialize(Scene &current_scene) = 0;
  virtual void Deserialize(Scene &current_scene, nlohmann::json &sys_json) = 0;

  std::string GetEntityIDString(entt::entity &e) {
    return std::to_string(static_cast<u32>(e));
  }
  entt::entity GetEntityIDFromString(const std::string &e) {
    return entt::entity{std::stoul(e)};
  }

  virtual ~ECSSystem() {}
};

class SystemManager {
public:
  std::vector<std::unique_ptr<ECSSystem>> systems;
  std::unordered_map<hash_string_ge, ECSSystem *> system_type_aliases;

  template <typename _SysType, typename... Args>
  ECSSystem *AddSystem(Args &&...args) {
    static_assert(std::is_base_of<ECSSystem, _SysType>());
    HashString type_str_hash = HashString{HashUtils::GetTypeHash<_SysType>()};
    systems.push_back(std::make_unique<_SysType>());
    system_type_aliases.emplace(type_str_hash, systems.back().get());
    return system_type_aliases[type_str_hash];
  }

  GEM_IMPL_ALLOC(SystemManager)
};
} // namespace gem