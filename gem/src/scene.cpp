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

scene::scene(const std::string &scene_name)
    : m_name(scene_name), m_name_hash(scene_name) {
  ZoneScoped;
}

entity scene::create_entity(const std::string &name) {
  ZoneScoped;
  entt::entity e = m_registry.create();
  m_registry.emplace<entity_data>(e, entity_data{name});
  p_created_entity_count++;
  return entity(this, e);
}

std::vector<entity> scene::create_entity_from_model(
    asset_handle model_asset_handle, model &model_to_load,
    asset_handle shader_asset_handle, gl_shader &material_shader, glm::vec3 scale,
    glm::vec3 euler, std::map<std::string, texture_map_type> known_maps) {
  ZoneScoped;
  std::vector<entity> entities{};
  for (u32 i = 0; i < model_to_load.m_meshes.size(); i++) {
    auto &entry = model_to_load.m_meshes[i];
    std::stringstream entity_name;
    entity_name << "Entity " << p_created_entity_count;
    entity e = create_entity(entity_name.str());

    transform &trans = e.add_component<transform>();
    trans.m_scale = scale;
    trans.m_euler = euler;
    e.add_component<mesh_component>(
        mesh_component{entry, model_asset_handle, i});
    material &current_mat =
        e.add_component<material>(shader_asset_handle, material_shader);

    GLenum texture_slot = GL_TEXTURE0;
    // go through each known map type
    for (auto &[uniform_name, map_type] : known_maps) {
      // check if material has desired map type
      model::material_entry &material_entry =
          model_to_load.m_materials[entry.m_material_index];
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

  // TODO: Update to work from scene overall aabb
  m_scene_bounding_volume = utils::transform_aabb(
      model_to_load.m_aabb,
      utils::get_model_matrix(glm::vec3(0.0), euler, scale));

  return entities;
}

bool scene::does_entity_exist(u32 index) {
  ZoneScoped;
  return m_registry.valid(entt::entity{index});
}

void scene::on_update() {
  ZoneScoped;
  for (auto &sys : engine::systems.m_systems) {
    sys->update(*this);
  }
}

void scene::update_aabb(aabb &in) {
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

entity::entity(scene *escene, entt::entity e) : m_scene(escene), m_handle(e) {
  ZoneScoped;
}

scene_manager::scene_manager() { ZoneScoped; }

void scene_manager::close_scene(hash_string scene_hash) { ZoneScoped; }

scene *scene_manager::get_scene(hash_string scene_hash) {
  ZoneScoped;

  for (auto &scene : p_active_scenes) {
    if (scene->m_name_hash == scene_hash) {
      return p_active_scenes[scene_hash].get();
    }
  }
  return nullptr;
}

scene *scene_manager::create_scene(const std::string &scene_name) {
  ZoneScoped;
  hash_string scene_hash(scene_name);
  scene *existing_scene = get_scene(scene_hash);
  if (existing_scene != nullptr) {
    return existing_scene;
  }

  p_active_scenes.emplace_back(std::make_unique<scene>(scene_name));
  return p_active_scenes.back().get();
}

scene *scene_manager::load_scene(nlohmann::json &scene_json) {
  ZoneScoped;
  std::string scene_name = scene_json["name"];
  p_active_scenes.push_back(std::make_unique<scene>(scene_name));
  scene *s = p_active_scenes.back().get();

  for (auto &sys : engine::systems.m_systems) {
    sys->deserialize(
        *s, scene_json["systems"][std::to_string(sys->m_sys_hash.m_value)]);
  }

  return s;
}

nlohmann::json scene_manager::save_scene(scene *ser_scene) {
  ZoneScoped;
  GEM_ASSERT(ser_scene, "Trying to save an invalid scene");

  nlohmann::json json{};

  json["name"] = ser_scene->m_name;
  json["systems"] = nlohmann::json();

  for (auto &sys : engine::systems.m_systems) {
    json["systems"][std::to_string(sys->m_sys_hash.m_value)] =
        sys->serialize(*ser_scene);
  }

  return json;
}
void  scene_manager::save_scene_to_disk(const std::string &path,
                                                 scene *ser_scene) {

  nlohmann::json scene_json = save_scene(ser_scene);
  utils::save_string_to_path(path, scene_json.dump());
}

scene *scene_manager::load_scene_from_disk(const std::string &scene_path) {
  std::string scene_json_str = utils::load_string_from_path(scene_path);
  if(scene_json_str.empty())
  {
    return nullptr;
  }
  nlohmann::json scene_json = nlohmann::json::parse(scene_json_str);
  GEM_ASSERT(!scene_json.empty(), "Failed to deserialize scene json at path : " + scene_path);
  return load_scene(scene_json);
}
} // namespace gem