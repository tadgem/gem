#include "gem/gl/gl_shader.h"
#include "gem/profile.h"
#include "gem/utils.h"
#include "gtc/type_ptr.hpp"
#include <array>
#include <iostream>
#include <sstream>

namespace gem {

GLShader::GLShader(const std::string &comp) {
  ZoneScoped;
  auto c = CompileShader(comp, GL_COMPUTE_SHADER);
  linked_program_id = LinkShader(c);

  glDeleteShader(c);
}

GLShader::GLShader(const std::string &vert, const std::string &frag) {
  ZoneScoped;
  auto v = CompileShader(vert, GL_VERTEX_SHADER);
  auto f = CompileShader(frag, GL_FRAGMENT_SHADER);

  linked_program_id = LinkShader(v, f);

  glDeleteShader(v);
  glDeleteShader(f);
}

GLShader::GLShader(const std::string &vert, const std::string &geom,
                     const std::string &frag) {
  ZoneScoped;
  auto v = CompileShader(vert, GL_VERTEX_SHADER);
  auto g = CompileShader(geom, GL_GEOMETRY_SHADER);
  auto f = CompileShader(frag, GL_FRAGMENT_SHADER);

  linked_program_id = LinkShader(v, g, f);

  glDeleteShader(v);
  glDeleteShader(g);
  glDeleteShader(f);
}

void GLShader::Use() {
  ZoneScoped;
  glUseProgram(linked_program_id);
}

void GLShader::Release() {
  ZoneScoped;
  glDeleteProgram(linked_program_id);
}

void GLShader::SetBool(const std::string &name, bool value) const {
  ZoneScoped;
  glUniform1i(glGetUniformLocation(linked_program_id, name.c_str()), (int)value);
}

void GLShader::SetInt(const std::string &name, int value) const {
  ZoneScoped;
  glUniform1i(glGetUniformLocation(linked_program_id, name.c_str()), value);
}

void GLShader::SetUint(const std::string &name, unsigned int value) const {
  ZoneScoped;
  glUniform1ui(glGetUniformLocation(linked_program_id, name.c_str()), value);
}

void GLShader::SetFloat(const std::string &name, float value) const {
  ZoneScoped;
  glUniform1f(glGetUniformLocation(linked_program_id, name.c_str()), value);
}

void GLShader::SetVec2f(const std::string &name, glm::vec2 value) const {
  ZoneScoped;
  glUniform2f(glGetUniformLocation(linked_program_id, name.c_str()), value.x,
              value.y);
}

void GLShader::SetVec3f(const std::string &name, glm::vec3 value) const {
  ZoneScoped;
  glUniform3f(glGetUniformLocation(linked_program_id, name.c_str()), value.x, value.y,
              value.z);
}

void GLShader::SetVec4f(const std::string &name, glm::vec4 value) const {
  ZoneScoped;
  glUniform4f(glGetUniformLocation(linked_program_id, name.c_str()), value.x, value.y,
              value.z, value.w);
}

void GLShader::SetVec2i(const std::string &name, glm::ivec2 value) const {
  ZoneScoped;
  glUniform2i(glGetUniformLocation(linked_program_id, name.c_str()), value.x,
              value.y);
}

void GLShader::SetVec3i(const std::string &name, glm::ivec3 value) const {
  ZoneScoped;
  glUniform3i(glGetUniformLocation(linked_program_id, name.c_str()), value.x, value.y,
              value.z);
}

void GLShader::SetVec4i(const std::string &name, glm::ivec4 value) const {
  ZoneScoped;
  glUniform4i(glGetUniformLocation(linked_program_id, name.c_str()), value.x, value.y,
              value.z, value.w);
}

void GLShader::SetMat3f(const std::string &name, glm::mat3 value) const {
  ZoneScoped;
  glUniformMatrix3fv(glGetUniformLocation(linked_program_id, name.c_str()), 1,
                     GL_FALSE, glm::value_ptr(value));
}

void GLShader::SetMat4f(const std::string &name, glm::mat4 value) const {
  ZoneScoped;
  glUniformMatrix4fv(glGetUniformLocation(linked_program_id, name.c_str()), 1,
                     GL_FALSE, glm::value_ptr(value));
}

gl_handle GLShader::CompileShader(const std::string &source,
                                    GLenum shader_stage) {
  ZoneScoped;
  const char *src = source.c_str();
  gl_handle s = glCreateShader(shader_stage);
  glShaderSource(s, 1, &src, NULL);
  glCompileShader(s);

  int success = 0;

  glGetShaderiv(s, GL_COMPILE_STATUS, &success);

  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(s, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
  };

  return s;
}

int GLShader::LinkShader(gl_handle comp) {
  ZoneScoped;
  auto prog_id = glCreateProgram();
  glAttachShader(prog_id, comp);
  glLinkProgram(prog_id);

  int success = 0;

  glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(prog_id, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  return prog_id;
}

int GLShader::LinkShader(gl_handle vert, gl_handle frag) {
  ZoneScoped;
  auto prog_id = glCreateProgram();
  glAttachShader(prog_id, vert);
  glAttachShader(prog_id, frag);
  glLinkProgram(prog_id);

  int success = 0;

  glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(prog_id, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  return prog_id;
}

int GLShader::LinkShader(gl_handle vert, gl_handle geom, gl_handle frag) {
  ZoneScoped;
  auto prog_id = glCreateProgram();
  glAttachShader(prog_id, vert);
  glAttachShader(prog_id, geom);
  glAttachShader(prog_id, frag);
  glLinkProgram(prog_id);

  int success = 0;

  glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(prog_id, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  return prog_id;
}

GLShader GLShader::CreateFromComposite(const std::string &composite_shader) {
  std::unordered_map<GLShader::Stage, std::string> stages =
      SplitCompositeShader(composite_shader);

  if (stages.find(GLShader::Stage::kCompute) != stages.end()) {
    return GLShader(stages[GLShader::Stage::kCompute]);
  }

  if (stages.find(GLShader::Stage::kVertex) != stages.end() &&
      stages.find(GLShader::Stage::kFragment) != stages.end() &&
      stages.find(GLShader::Stage::kGeometry) == stages.end()) {
    return GLShader(stages[GLShader::Stage::kVertex],
                     stages[GLShader::Stage::kFragment]);
  }

  if (stages.find(GLShader::Stage::kVertex) != stages.end() &&
      stages.find(GLShader::Stage::kFragment) != stages.end() &&
      stages.find(GLShader::Stage::kGeometry) != stages.end()) {
    return GLShader(stages[GLShader::Stage::kVertex],
                     stages[GLShader::Stage::kGeometry],
                     stages[GLShader::Stage::kFragment]);
  }
  return {};
}

GLShader::UniformType GLShader::GetUniformTypeFromGL(GLenum type) {
  ZoneScoped;
  switch (type) {
  case GL_SAMPLER_2D:
    return UniformType::kSampler2D;
  case GL_SAMPLER_3D:
    return UniformType::kSampler3D;
  case GL_IMAGE_2D:
    return UniformType::kImage2D;
  case GL_IMAGE_3D:
    return UniformType::kImage3D;
  case GL_INT:
    return UniformType::kInt;
  case GL_FLOAT:
    return UniformType::kFloat;
  case GL_FLOAT_VEC2:
    return UniformType::kVec2;
  case GL_FLOAT_VEC3:
    return UniformType::kVec3;
  case GL_FLOAT_VEC4:
    return UniformType::kVec4;
  case GL_FLOAT_MAT3:
    return UniformType::kMat3;
  case GL_FLOAT_MAT4:
    return UniformType::kMat4;
  default:
    return UniformType::kUnknown;
  }

  return UniformType::kUnknown;
}
} // namespace gem