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
  auto c = compile_shader(comp, GL_COMPUTE_SHADER);
  m_shader_id = link_shader(c);

  glDeleteShader(c);
}

GLShader::GLShader(const std::string &vert, const std::string &frag) {
  ZoneScoped;
  auto v = compile_shader(vert, GL_VERTEX_SHADER);
  auto f = compile_shader(frag, GL_FRAGMENT_SHADER);

  m_shader_id = link_shader(v, f);

  glDeleteShader(v);
  glDeleteShader(f);
}

GLShader::GLShader(const std::string &vert, const std::string &geom,
                     const std::string &frag) {
  ZoneScoped;
  auto v = compile_shader(vert, GL_VERTEX_SHADER);
  auto g = compile_shader(geom, GL_GEOMETRY_SHADER);
  auto f = compile_shader(frag, GL_FRAGMENT_SHADER);

  m_shader_id = link_shader(v, g, f);

  glDeleteShader(v);
  glDeleteShader(g);
  glDeleteShader(f);
}

void GLShader::use() {
  ZoneScoped;
  glUseProgram(m_shader_id);
}

void GLShader::release() {
  ZoneScoped;
  glDeleteProgram(m_shader_id);
}

void GLShader::set_bool(const std::string &name, bool value) const {
  ZoneScoped;
  glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), (int)value);
}

void GLShader::set_int(const std::string &name, int value) const {
  ZoneScoped;
  glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void GLShader::set_uint(const std::string &name, unsigned int value) const {
  ZoneScoped;
  glUniform1ui(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void GLShader::set_float(const std::string &name, float value) const {
  ZoneScoped;
  glUniform1f(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void GLShader::set_vec2(const std::string &name, glm::vec2 value) const {
  ZoneScoped;
  glUniform2f(glGetUniformLocation(m_shader_id, name.c_str()), value.x,
              value.y);
}

void GLShader::set_vec3(const std::string &name, glm::vec3 value) const {
  ZoneScoped;
  glUniform3f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y,
              value.z);
}

void GLShader::set_vec4(const std::string &name, glm::vec4 value) const {
  ZoneScoped;
  glUniform4f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y,
              value.z, value.w);
}

void GLShader::set_ivec2(const std::string &name, glm::ivec2 value) const {
  ZoneScoped;
  glUniform2i(glGetUniformLocation(m_shader_id, name.c_str()), value.x,
              value.y);
}

void GLShader::set_ivec3(const std::string &name, glm::ivec3 value) const {
  ZoneScoped;
  glUniform3i(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y,
              value.z);
}

void GLShader::set_ivec4(const std::string &name, glm::ivec4 value) const {
  ZoneScoped;
  glUniform4i(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y,
              value.z, value.w);
}

void GLShader::set_mat3(const std::string &name, glm::mat3 value) const {
  ZoneScoped;
  glUniformMatrix3fv(glGetUniformLocation(m_shader_id, name.c_str()), 1,
                     GL_FALSE, glm::value_ptr(value));
}

void GLShader::set_mat4(const std::string &name, glm::mat4 value) const {
  ZoneScoped;
  glUniformMatrix4fv(glGetUniformLocation(m_shader_id, name.c_str()), 1,
                     GL_FALSE, glm::value_ptr(value));
}

gl_handle GLShader::compile_shader(const std::string &source,
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

int GLShader::link_shader(gl_handle comp) {
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

int GLShader::link_shader(gl_handle vert, gl_handle frag) {
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

int GLShader::link_shader(gl_handle vert, gl_handle geom, gl_handle frag) {
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

std::unordered_map<GLShader::stage, std::string>
GLShader::split_composite_shader(const std::string &input) {
  ZoneScoped;
  static std::unordered_map<std::string, GLShader::stage> s_known_stages = {
      {"#frag", GLShader::stage::fragment},
      {"#vert", GLShader::stage::vertex},
      {"#geom", GLShader::stage::geometry},
      {"#compute", GLShader::stage::compute}};
  // static std::unordered_map<std::string, gl_shader::stage> s_known_stages = {
  // };
  auto stages = std::unordered_map<GLShader::stage, std::string>();
  std::string version = "";

  std::stringstream input_stream(input);
  std::stringstream stage_stream{};
  std::string line = "";
  std::string stage = "";

  while (std::getline(input_stream, line)) {
    if (line.find("#version") != std::string::npos) {
      version = line;
      stage_stream << version << "\n";
      continue;
    }

    for (auto &[known, stage_enum] : s_known_stages) {
      if (line.find(known) != std::string::npos) {
        if (!stage.empty()) {
          stages.emplace(GLShader::stage(s_known_stages[stage]),
                         std::string(stage_stream.str()));
          stage_stream.str(std::string());
          stage_stream.clear();
          stage_stream << version << "\n";
        }

        stage = known;
        break;
      }
    }

    if (line == stage) {
      continue;
    }

    stage_stream << line << "\n";
  }

  std::string last_stream = stage_stream.str();
  if (!stage.empty() && !last_stream.empty()) {
    stages.emplace(s_known_stages[stage], last_stream);
  }

  return stages;
}

GLShader GLShader::create_from_composite(const std::string &composite_shader) {
  std::unordered_map<GLShader::stage, std::string> stages =
      GLShader::split_composite_shader(composite_shader);

  if (stages.find(GLShader::stage::compute) != stages.end()) {
    return GLShader(stages[GLShader::stage::compute]);
  }

  if (stages.find(GLShader::stage::vertex) != stages.end() &&
      stages.find(GLShader::stage::fragment) != stages.end() &&
      stages.find(GLShader::stage::geometry) == stages.end()) {
    return GLShader(stages[GLShader::stage::vertex],
                     stages[GLShader::stage::fragment]);
  }

  if (stages.find(GLShader::stage::vertex) != stages.end() &&
      stages.find(GLShader::stage::fragment) != stages.end() &&
      stages.find(GLShader::stage::geometry) != stages.end()) {
    return GLShader(stages[GLShader::stage::vertex],
                     stages[GLShader::stage::geometry],
                     stages[GLShader::stage::fragment]);
  }
  return {};
}

GLShader::uniform_type GLShader::get_type_from_gl(GLenum type) {
  ZoneScoped;
  switch (type) {
  case GL_SAMPLER_2D:
    return uniform_type::sampler2D;
  case GL_SAMPLER_3D:
    return uniform_type::sampler3D;
  case GL_IMAGE_2D:
    return uniform_type::image2D;
  case GL_IMAGE_3D:
    return uniform_type::image3D;
  case GL_INT:
    return uniform_type::_int;
  case GL_FLOAT:
    return uniform_type::_float;
  case GL_FLOAT_VEC2:
    return uniform_type::vec2;
  case GL_FLOAT_VEC3:
    return uniform_type::vec3;
  case GL_FLOAT_VEC4:
    return uniform_type::vec4;
  case GL_FLOAT_MAT3:
    return uniform_type::mat3;
  case GL_FLOAT_MAT4:
    return uniform_type::mat4;
  default:
    return uniform_type::UNKNOWN;
  }

  return uniform_type::UNKNOWN;
}
} // namespace gem