#pragma once
#include "gem/asset.h"
#include "gem/dbg_memory.h"
#include "gem/mesh.h"
#include "gem/texture.h"
#include <string>
#include <unordered_map>

namespace gem {

class Model {
public:
  struct MeshEntry {
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
    std::vector<u32> m_indices;
    AABB m_mesh_aabb;
    u32 m_material_index;

    GEM_IMPL_ALLOC(MeshEntry)
  };

  struct MaterialEntry {
    std::unordered_map<TextureMapType, TextureEntry> m_material_maps;
    GEM_IMPL_ALLOC(MaterialEntry);
  };

  std::vector<AMesh*> m_meshes;
  std::vector<MaterialEntry> m_materials;
  AABB m_aabb;

  static Model load_model_and_textures_from_path(const std::string &path);
  static Model
  load_model_from_path_entries(const std::string &path,
                               std::vector<TextureEntry> &texture_entries,
                               std::vector<MeshEntry> &mesh_entries);

  void update_aabb();
  void release();

  GEM_IMPL_ALLOC(Model);
};

using ModelAsset = TAsset<Model, AssetType::model>;
} // namespace gem