#pragma once
#include <string>
#include <unordered_map>

namespace gem {
  class Shader {
  public:
    enum class stage { UNKNOWN, vertex, fragment, geometry, compute };
    enum class uniform_type {
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

    static std::unordered_map<Shader::stage, std::string>
    split_composite_shader(const std::string &input);

  };
}