#pragma once
#include "entt.hpp"
#include "gem/alias.h"
#include "gem/shape.h"
#include "gem/texture.h"
#include "json.hpp"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace gem {

class entity;
class model;
class shader;

class scene {
public:
  scene(const std::string &scene_name);

  entity create_entity(const std::string &name);
  std::vector<entity> create_entity_from_model(
      asset_handle model_asset_handle, model &model_to_load,
      asset_handle shader_asset_handle, shader &material_shader,
      glm::vec3 scale = glm::vec3(1.0f), glm::vec3 euler = glm::vec3(0.0f),
      std::map<std::string, texture_map_type> known_maps = {});

  bool does_entity_exist(u32 e);

  void on_update();
  void update_aabb(aabb &in);

  const std::string m_name;
  const hash_string m_name_hash;
  entt::registry m_registry;
  aabb m_scene_bounding_volume;

protected:
  u32 p_created_entity_count;
};

class entity {
public:
  entt::entity m_handle;
  scene *m_scene;

  entity(scene *escene, entt::entity e);

  static entity INVALID() {
    return entity(nullptr, static_cast<entt::entity>(UINT32_MAX));
  }

  template <typename _Ty, typename... Args> _Ty &add_component(Args &&...args) {
    return m_scene->m_registry.emplace<_Ty>(m_handle,
                                            std::forward<Args>(args)...);
  }

  template <typename _Ty> _Ty &get_component() {
    return m_scene->m_registry.get<_Ty>(m_handle);
  }

  template <typename _Ty> std::optional<_Ty> try_get_component() {
    if (!m_scene->m_registry.any_of<_Ty>(m_handle)) {
      return {};
    }
    return get_component<_Ty>();
  }

  template <typename _Ty> bool has_component() {
    if (!m_scene->m_registry.any_of<_Ty>(m_handle)) {
      return false;
    }
    return true;
  }
};

// component which all entities must have
struct entity_data {
  std::string m_name;
  entity m_parent = entity::INVALID();
};

class scene_manager {
public:
  scene_manager();

  scene *create_scene(const std::string &name);
  scene *load_scene(nlohmann::json &scene_json);
  scene *get_scene(hash_string scene_hash);
  void close_scene(hash_string scene_hash);
  nlohmann::json save_scene(scene *ser_scene);

protected:
  std::vector<std::unique_ptr<scene>> p_active_scenes;
};
} // namespace gem