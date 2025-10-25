#pragma once
#include "GL/glew.h"
#include "gem/alias.h"
#include "gem/shader.h"
#include "glm.hpp"


namespace gem {

class GLShader : public Shader {
public:

  unsigned int linked_program_id;

  GLShader() = default;
  GLShader(const std::string &comp);
  GLShader(const std::string &vert, const std::string &frag);
  GLShader(const std::string &vert, const std::string &geom,
            const std::string &frag);

  void Use();
  void Release();

  void SetBool(const std::string &name, bool value) const;
  void SetInt(const std::string &name, int value) const;
  void SetUint(const std::string &name, unsigned int value) const;
  void SetFloat(const std::string &name, float value) const;
  void SetVec2f(const std::string &name, glm::vec2 value) const;
  void SetVec3f(const std::string &name, glm::vec3 value) const;
  void SetVec4f(const std::string &name, glm::vec4 value) const;
  void SetVec2i(const std::string &name, glm::ivec2 value) const;
  void SetVec3i(const std::string &name, glm::ivec3 value) const;
  void SetVec4i(const std::string &name, glm::ivec4 value) const;
  void SetMat3f(const std::string &name, glm::mat3 value) const;
  void SetMat4f(const std::string &name, glm::mat4 value) const;

  static gl_handle CompileShader(const std::string &source,
                                  GLenum shader_stage);
  static int LinkShader(gl_handle comp);
  static int LinkShader(gl_handle vert, gl_handle frag);
  static int LinkShader(gl_handle vert, gl_handle geom, gl_handle frag);


  static GLShader CreateFromComposite(const std::string &composite_shader);

  static UniformType GetUniformTypeFromGL(GLenum type);
};
} // namespace gem