#pragma once
#include <string>
#include <unordered_map>

namespace gem {
  class Shader {
  public:
    enum class Stage { kUnknown, kVertex, kFragment, kGeometry, kCompute };
    enum class UniformType {
      kUnknown,
      kInt,
      kFloat,
      kVec2,
      kVec3,
      kVec4,
      kMat3,
      kMat4,
      kSampler2D,
      kSampler3D,
      kImage2D,
      kImage3D
    };

    static std::unordered_map<Shader::Stage, std::string>
    SplitCompositeShader(const std::string &input);

  };
}