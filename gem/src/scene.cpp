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

Entity Scene::create_entity(const std::string &name) {
  ZoneScoped;
  entt::entity e = m_registry.create();
  m_registry.emplace<EntityData>(e, EntityData{name});
  p_created_entity_count++;
  return Entity(this, e);
}

std::vector<Entity> Scene::create_entity_from_model(
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
    Entity e = create_entity(entity_name.str());

    Transform &trans = e.add_component<Transform>();
    trans.m_scale = scale;
    trans.m_euler = euler;
    e.add_component<MeshComponent>(MeshComponent{entry, model_asset_handle, i});
    Material &current_mat =
        e.add_component<Material>(shader_asset_handle, material_shader);

    GLenum texture_slot = GL_TEXTURE0;
    // go through each known map type
    for (auto &[uniform_name, map_type] : known_maps) {
      // check if material has desired map type
      Model::MaterialEntry &material_entry =
          model_to_load.m_materials[entry->m_material_index];
      if (material_entry.m_material_maps.find(map_type) !=
          material_entry.m_material_maps.end()) {
        current_mat.set_sampler(uniform_name, texture_slot,
                                material_entry.m_material_maps[map_type],
                                GL_TEXTURE_2D);

        texture_slot++;
      }
    }
    entities.push_back(e);
  }
  static const glm::vec3 ZERO = glm::vec3(0.0);
  glm::mat4 model_matrix = Utils::get_model_matrix(ZERO, euler, scale);

  // TODO: Update to work from scene overall aabb
  m_scene_bounding_volume = Utils::transform_aabb(
      model_to_load.m_aabb, model_matrix);

  return entities;
}

bool Scene::does_entity_exist(u32 index) {
  ZoneScoped;
  return m_registry.valid(entt::entity{index});
}

void Scene::on_update() {
  ZoneScoped;
  for (auto &sys : Engine::systems.m_systems) {
    sys->update(*this);
  }
}

void Scene::update_aabb(AABB &in) {
  ZoneScoped;
  if (in.m_min.x < m_scene_bounding_volume.m_min.x) {
    m_scene_bounding_volume.m_min.x = in.m_min.x;
  }
  if (in.m_min.y < m_scene_bounding_volume.m_min.y) {
    m_scene_bounding_volume.m_min.y = in.m_min.y;
  }
  if (in.m_min.z < m_scene_bounding_volume.m_min.z) {
    m_scene_bounding_volume.m_min.z = in.m_min.z;
  }

  if (in.m_max.x > m_scene_bounding_volume.m_max.x) {
    m_scene_bounding_volume.m_max.x = in.m_max.x;
  }
  if (in.m_max.y > m_scene_bounding_volume.m_max.y) {
    m_scene_bounding_volume.m_max.y = in.m_max.y;
  }
  if (in.m_max.z > m_scene_bounding_volume.m_max.z) {
    m_scene_bounding_volume.m_max.z = in.m_max.z;
  }
}

Entity::Entity(Scene *escene, entt::entity e) : m_scene(escene), m_handle(e) {
  ZoneScoped;
}

SceneManager::SceneManager() { ZoneScoped; }

void SceneManager::close_scene(HashString scene_hash) { ZoneScoped; }

Scene *SceneManager::get_scene(HashString scene_hash) {
  ZoneScoped;

  for (auto &scene : p_active_scenes) {
    if (scene->m_name_hash == scene_hash) {
      return p_active_scenes[scene_hash].get();
    }
  }
  return nullptr;
}

Scene *SceneManager::create_scene(const std::string &scene_name) {
  ZoneScoped;
  HashString scene_hash(scene_name);
  Scene *existing_scene = get_scene(scene_hash);
  if (existing_scene != nullptr) {
    return existing_scene;
  }

  p_active_scenes.emplace_back(std::make_unique<Scene>(scene_name));
  return p_active_scenes.back().get();
}

Scene *SceneManager::load_scene(nlohmann::json &scene_json) {
  ZoneScoped;
  std::string scene_name = scene_json["name"];
  p_active_scenes.push_back(std::make_unique<Scene>(scene_name));
  Scene *s = p_active_scenes.back().get();

  for (auto &sys : Engine::systems.m_systems) {
    sys->deserialize(
        *s, scene_json["systems"][std::to_string(sys->m_sys_hash.m_value)]);
  }

  return s;
}

nlohmann::json SceneManager::save_scene(Scene *ser_scene) {
  ZoneScoped;
  GEM_ASSERT(ser_scene, "Trying to save an invalid scene");

  nlohmann::json json{};

  json["name"] = ser_scene->m_name;
  json["systems"] = nlohmann::json();

  for (auto &sys : Engine::systems.m_systems) {
    json["systems"][std::to_string(sys->m_sys_hash.m_value)] =
        sys->serialize(*ser_scene);
  }

  return json;
}
void SceneManager::save_scene_to_disk(const std::string &path,
                                       Scene *ser_scene) {

  nlohmann::json scene_json = save_scene(ser_scene);
  Utils::save_string_to_path(path, scene_json.dump());
}

Scene *SceneManager::load_scene_from_disk(const std::string &scene_path) {
  std::string scene_json_str = Utils::load_string_from_path(scene_path);
  if (scene_json_str.empty()) {
    return nullptr;
  }
  nlohmann::json scene_json = nlohmann::json::parse(scene_json_str);
  GEM_ASSERT(!scene_json.empty(),
             "Failed to deserialize scene json at path : " + scene_path);
  return load_scene(scene_json);
}
} // namespace gem