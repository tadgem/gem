#pragma once
#include <string>
#include <unordered_map>

namespace gem {
  class Shader {
  public:
    enum class Stage { UNKNOWN, vertex, fragment, geometry, compute };
    enum class UniformType {
      UNKNOWN,
      _int,
      _float,
      vec2,
      vec3,
      vec4,
      mat3,
      mat4,
      sampler2D,
      sampler3D,
      image2D,
      image3D
    };

    static std::unordered_map<Shader::Stage, std::string>
    SplitCompositeShader(const std::string &input);

  };
}