#define GLM_ENABLE_EXPERIMENTAL

#include "gem/scene.h"
#include "gem/dbg_assert.h"
#include "gem/engine.h"
#include "gem/gl/gl_shader.h"
#include "gem/material.h"
#include "gem/model.h"
#include "gem/profile.h"
#include "gem/transform.h"
#include "gem/utils.h"
#include <sstream>

namespace gem {

Scene::Scene(const std::string &scene_name)
    : m_name(scene_name), m_name_hash(scene_name) {
  ZoneScoped;
}

Entity Scene::CreateEntity(const std::string &name) {
  ZoneScoped;
  entt::entity e = m_registry.create();
  m_registry.emplace<EntityData>(e, EntityData{name});
  p_created_entity_count++;
  return Entity(this, e);
}

std::vector<Entity> Scene::CreateEntityFromModel(
    AssetHandle model_asset_handle, Model &model_to_load,
    AssetHandle shader_asset_handle, GLShader &material_shader,
    glm::vec3 scale, glm::vec3 euler,
    std::map<std::string, TextureMapType> known_maps) {
  ZoneScoped;
  std::vector<Entity> entities{};
  for (u32 i = 0; i < model_to_load.m_meshes.size(); i++) {
    auto &entry = model_to_load.m_meshes[i];
    std::stringstream entity_name;
    entity_name << "Entity " << p_created_entity_count;
    Entity e = CreateEntity(entity_name.str());

    Transform &trans = e.AddComponent<Transform>();
    trans.m_scale = scale;
    trans.m_euler = euler;
    e.AddComponent<MeshComponent>(MeshComponent{entry, model_asset_handle, i});
    Material &current_mat =
        e.AddComponent<Material>(shader_asset_handle, material_shader);

    GLenum texture_slot = GL_TEXTURE0;
    // go through each known map type
    for (auto &[uniform_name, map_type] : known_maps) {
      // check if material has desired map type
      Model::MaterialEntry &material_entry =
          model_to_load.m_materials[entry->m_material_index];
      if (material_entry.m_material_maps.find(map_type) !=
          material_entry.m_material_maps.end()) {
        current_mat.SetSampler(uniform_name, texture_slot,
                                material_entry.m_material_maps[map_type],
                                GL_TEXTURE_2D);

        texture_slot++;
      }
    }
    entities.push_back(e);
  }
  static const glm::vec3 ZERO = glm::vec3(0.0);
  glm::mat4 model_matrix = Utils::GetModelMatrix(ZERO, euler, scale);

  // TODO: Update to work from scene overall aabb
  m_scene_bounding_volume = Utils::TransformAABB(
      model_to_load.m_aabb, model_matrix);

  return entities;
}

bool Scene::DoesEntityExist(u32 index) {
  ZoneScoped;
  return m_registry.valid(entt::entity{index});
}

void Scene::Update() {
  ZoneScoped;
  for (auto &sys : Engine::systems.systems) {
    sys->Update(*this);
  }
}

void Scene::UpdateAABB(AABB &in) {
  ZoneScoped;
  if (in.min.x < m_scene_bounding_volume.min.x) {
    m_scene_bounding_volume.min.x = in.min.x;
  }
  if (in.min.y < m_scene_bounding_volume.min.y) {
    m_scene_bounding_volume.min.y = in.min.y;
  }
  if (in.min.z < m_scene_bounding_volume.min.z) {
    m_scene_bounding_volume.min.z = in.min.z;
  }

  if (in.max.x > m_scene_bounding_volume.max.x) {
    m_scene_bounding_volume.max.x = in.max.x;
  }
  if (in.max.y > m_scene_bounding_volume.max.y) {
    m_scene_bounding_volume.max.y = in.max.y;
  }
  if (in.max.z > m_scene_bounding_volume.max.z) {
    m_scene_bounding_volume.max.z = in.max.z;
  }
}

Entity::Entity(Scene *escene, entt::entity e) : m_scene(escene), m_handle(e) {
  ZoneScoped;
}

SceneManager::SceneManager() { ZoneScoped; }

void SceneManager::CloseScene(HashString scene_hash) { ZoneScoped; }

Scene *SceneManager::GetScene(HashString scene_hash) {
  ZoneScoped;

  for (auto &scene : p_active_scenes) {
    if (scene->m_name_hash == scene_hash) {
      return p_active_scenes[scene_hash].get();
    }
  }
  return nullptr;
}

Scene *SceneManager::CreateScene(const std::string &scene_name) {
  ZoneScoped;
  HashString scene_hash(scene_name);
  Scene *existing_scene = GetScene(scene_hash);
  if (existing_scene != nullptr) {
    return existing_scene;
  }

  p_active_scenes.emplace_back(std::make_unique<Scene>(scene_name));
  return p_active_scenes.back().get();
}

Scene *SceneManager::LoadScene(nlohmann::json &scene_json) {
  ZoneScoped;
  std::string scene_name = scene_json["name"];
  p_active_scenes.push_back(std::make_unique<Scene>(scene_name));
  Scene *s = p_active_scenes.back().get();

  for (auto &sys : Engine::systems.systems) {
    sys->Deserialize(
        *s, scene_json["systems"][std::to_string(sys->kSysHash.m_value)]);
  }

  return s;
}

nlohmann::json SceneManager::SaveScene(Scene *ser_scene) {
  ZoneScoped;
  GEM_ASSERT(ser_scene, "Trying to save an invalid scene");

  nlohmann::json json{};

  json["name"] = ser_scene->m_name;
  json["systems"] = nlohmann::json();

  for (auto &sys : Engine::systems.systems) {
    json["systems"][std::to_string(sys->kSysHash.m_value)] =
        sys->Serialize(*ser_scene);
  }

  return json;
}
void SceneManager::SaveSceneToPath(const std::string &path,
                                       Scene *ser_scene) {

  nlohmann::json scene_json = SaveScene(ser_scene);
  Utils::SaveStringToPath(path, scene_json.dump());
}

Scene *SceneManager::LoadSceneFromPath(const std::string &scene_path) {
  std::string scene_json_str = Utils::LoadStringFromPath(scene_path);
  if (scene_json_str.empty()) {
    return nullptr;
  }
  nlohmann::json scene_json = nlohmann::json::parse(scene_json_str);
  GEM_ASSERT(!scene_json.empty(),
             "Failed to deserialize scene json at path : " + scene_path);
  return LoadScene(scene_json);
}
} // namespace gem