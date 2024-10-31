#pragma once
#include "gem/asset.h"
#include "gem/dbg_memory.h"
#include "gem/mesh.h"
#include "gem/texture.h"
#include <string>
#include <unordered_map>

namespace gem {

class model {
public:
  struct mesh_entry {
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
    std::vector<u32> m_indices;
    aabb m_mesh_aabb;
    u32 m_material_index;

    DEBUG_IMPL_ALLOC(mesh_entry)
  };

  struct material_entry {
    std::unordered_map<texture_map_type, texture_entry> m_material_maps;
    DEBUG_IMPL_ALLOC(material_entry);
  };

  std::vector<mesh> m_meshes;
  std::vector<material_entry> m_materials;
  aabb m_aabb;

  static model load_model_and_textures_from_path(const std::string &path);
  static model
  load_model_from_path_entries(const std::string &path,
                               std::vector<texture_entry> &texture_entries,
                               std::vector<mesh_entry> &mesh_entries);

  void update_aabb();
  void release();

  DEBUG_IMPL_ALLOC(model);
};

using model_asset = asset_t<model, asset_type::model>;
} // namespace gem