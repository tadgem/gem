#pragma once
#include "entt.hpp"
#include "gem/AABB.h"
#include "gem/alias.h"
#include "gem/dbg_memory.h"
#include "gem/texture.h"
#include "json.hpp"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace gem {

class Entity;
class Model;
class GLShader;

class Scene {
public:
  Scene(const std::string &scene_name);

  Entity CreateEntity(const std::string &name);
  std::vector<Entity> CreateEntityFromModel(
      AssetHandle model_asset_handle, Model &model_to_load,
      AssetHandle shader_asset_handle, GLShader &material_shader,
      glm::vec3 scale = glm::vec3(1.0f), glm::vec3 euler = glm::vec3(0.0f),
      std::map<std::string, TextureMapType> known_maps = {});

  bool DoesEntityExist(u32 e);

  void Update();
  void UpdateAABB(AABB &in);

  const std::string m_name;
  const HashString m_name_hash;
  entt::registry m_registry;
  AABB m_scene_bounding_volume;

  GEM_IMPL_ALLOC(Scene)

protected:
  u32 p_created_entity_count;
};

class Entity {
public:
  entt::entity m_handle;
  Scene *m_scene;

  Entity(Scene *escene, entt::entity e);

  static Entity INVALID() {
    return Entity(nullptr, static_cast<entt::entity>(UINT32_MAX));
  }

  template <typename _Ty, typename... Args> _Ty &AddComponent(Args &&...args) {
    return m_scene->m_registry.emplace<_Ty>(m_handle,
                                            std::forward<Args>(args)...);
  }

  template <typename _Ty> _Ty &GetComponent() {
    return m_scene->m_registry.get<_Ty>(m_handle);
  }

  template <typename _Ty> std::optional<_Ty> TryGetComponent() {
    if (!m_scene->m_registry.any_of<_Ty>(m_handle)) {
      return {};
    }
    return GetComponent<_Ty>();
  }

  template <typename _Ty> bool HasComponent() {
    if (!m_scene->m_registry.any_of<_Ty>(m_handle)) {
      return false;
    }
    return true;
  }

  GEM_IMPL_ALLOC(Entity)
};

// component which all entities must have
struct EntityData {
  std::string m_name;
  Entity m_parent = Entity::INVALID();
};

class SceneManager {
public:
  SceneManager();

  Scene *           CreateScene(const std::string &name);
  Scene *           LoadScene(nlohmann::json &scene_json);
  Scene *           LoadSceneFromPath(const std::string &scene_path);
  Scene *           GetScene(HashString scene_hash);
  void              CloseScene(HashString scene_hash);
  nlohmann::json    SaveScene(Scene *ser_scene);
  void              SaveSceneToPath(const std::string &path, Scene *ser_scene);

  GEM_IMPL_ALLOC(SceneManager)
protected:
  std::vector<std::unique_ptr<Scene>> p_active_scenes;
};
} // namespace gem