#pragma once
#include "GL/glew.h"
#include "gem/alias.h"
#include "gem/shader.h"
#include "glm.hpp"


namespace gem {

class GLShader : public Shader {
public:

  unsigned int m_shader_id;

  GLShader() = default;
  GLShader(const std::string &comp);
  GLShader(const std::string &vert, const std::string &frag);
  GLShader(const std::string &vert, const std::string &geom,
            const std::string &frag);

  void use();
  void release();

  void set_bool(const std::string &name, bool value) const;
  void set_int(const std::string &name, int value) const;
  void set_uint(const std::string &name, unsigned int value) const;
  void set_float(const std::string &name, float value) const;
  void set_vec2(const std::string &name, glm::vec2 value) const;
  void set_vec3(const std::string &name, glm::vec3 value) const;
  void set_vec4(const std::string &name, glm::vec4 value) const;
  void set_ivec2(const std::string &name, glm::ivec2 value) const;
  void set_ivec3(const std::string &name, glm::ivec3 value) const;
  void set_ivec4(const std::string &name, glm::ivec4 value) const;
  void set_mat3(const std::string &name, glm::mat3 value) const;
  void set_mat4(const std::string &name, glm::mat4 value) const;

  static gl_handle compile_shader(const std::string &source,
                                  GLenum shader_stage);
  static int link_shader(gl_handle comp);
  static int link_shader(gl_handle vert, gl_handle frag);
  static int link_shader(gl_handle vert, gl_handle geom, gl_handle frag);


  static GLShader create_from_composite(const std::string &composite_shader);

  static uniform_type get_type_from_gl(GLenum type);
};
} // namespace gem