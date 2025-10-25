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

  const std::string scene_name;
  const HashString scene_name_hash;
  entt::registry registry;
  AABB aabb;

  GEM_IMPL_ALLOC(Scene)

protected:
  u32 created_entity_count_;
};

class Entity {
public:
  entt::entity handle;
  Scene *scene;

  Entity(Scene *escene, entt::entity e);

  static Entity INVALID() {
    return Entity(nullptr, static_cast<entt::entity>(UINT32_MAX));
  }

  template <typename _Ty, typename... Args> _Ty &AddComponent(Args &&...args) {
    return scene->registry.emplace<_Ty>(handle,
                                            std::forward<Args>(args)...);
  }

  template <typename _Ty> _Ty &GetComponent() {
    return scene->registry.get<_Ty>(handle);
  }

  template <typename _Ty> std::optional<_Ty> TryGetComponent() {
    if (!scene->registry.any_of<_Ty>(handle)) {
      return {};
    }
    return GetComponent<_Ty>();
  }

  template <typename _Ty> bool HasComponent() {
    if (!scene->registry.any_of<_Ty>(handle)) {
      return false;
    }
    return true;
  }

  GEM_IMPL_ALLOC(Entity)
};

// component which all entities must have
struct EntityData {
  std::string entity_name;
  Entity parent = Entity::INVALID();
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
  std::vector<std::unique_ptr<Scene>> active_scenes_;
};
} // namespace gem