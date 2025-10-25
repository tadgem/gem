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
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<u32> indices;
    AABB mesh_aabb;
    u32 material_index;

    GEM_IMPL_ALLOC(MeshEntry)
  };

  struct MaterialEntry {
    std::unordered_map<TextureMapType, TextureEntry> material_maps;
    GEM_IMPL_ALLOC(MaterialEntry);
  };

  std::vector<AMesh*> meshes;
  std::vector<MaterialEntry> materials;
  AABB aabb;

  static Model LoadModelAndTextures(const std::string &path);
  static Model
  LoadModelAndTextureEntries(const std::string &path,
                               std::vector<TextureEntry> &texture_entries,
                               std::vector<MeshEntry> &mesh_entries);

  void UpdateAABB();
  void Release();

  GEM_IMPL_ALLOC(Model);
};

using ModelAsset = TAsset<Model, AssetType::kModel>;
} // namespace gem