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
  const HashString m_sys_hash;

  ECSSystem(HashString sys_hash) : m_sys_hash(sys_hash) {}

  virtual void init() = 0;
  virtual void update(Scene &current_scene) = 0;
  virtual void cleanup() = 0;

  virtual nlohmann::json serialize(Scene &current_scene) = 0;
  virtual void deserialize(Scene &current_scene, nlohmann::json &sys_json) = 0;

  std::string get_entity_string(entt::entity &e) {
    return std::to_string(static_cast<u32>(e));
  }
  entt::entity get_entity_from_string(const std::string &e) {
    return entt::entity{std::stoul(e)};
  }

  virtual ~ECSSystem() {}
};

class SystemManager {
public:
  std::vector<std::unique_ptr<ECSSystem>> m_systems;
  std::unordered_map<hash_string_ge, ECSSystem *> m_system_type_aliases;

  template <typename _SysType, typename... Args>
  ECSSystem *add_system(Args &&...args) {
    static_assert(std::is_base_of<ECSSystem, _SysType>());
    HashString type_str_hash = HashString{HashUtils::get_type_hash<_SysType>()};
    m_systems.push_back(std::make_unique<_SysType>());
    m_system_type_aliases.emplace(type_str_hash, m_systems.back().get());
    return m_system_type_aliases[type_str_hash];
  }

  GEM_IMPL_ALLOC(SystemManager)
};
} // namespace gem