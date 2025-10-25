#pragma once
#include "gem/mesh.h"

namespace gem {

class AssetManager;

class Shapes {
public:
  inline static VAO kScreenQuad;
  inline static VAO kCubePositionOnly;

  inline static VAO kCubeVAO;
  inline static AMesh* kCubeMesh;

  inline static VAO kSphereVAO;
  inline static AMesh* kSphereMesh;

  inline static VAO kCylinderVAO;
  inline static AMesh* kCylinderMesh;

  inline static VAO kConeVAO;
  inline static AMesh* kConeMesh;

  inline static VAO kTorusVAO;
  inline static AMesh* kTorusMesh;

  static void InitBuiltInAssets(AssetManager &am, GPUBackend* backend);

  static VAO GenerateInstancedCube(std::vector<glm::mat4> &matrices,
                                    std::vector<glm::vec3> &uvs);
};
} // namespace gem