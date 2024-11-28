#pragma once
#include "gem/ecs_system.h"
#include "gem/gl/gl_shader.h"
#include "gem/model.h"
#include "gem/texture.h"
#include <any>
#include <map>
#include <string>
#define ENABLE_MATERIAL_UNIFORM_CHECKS
namespace gem {

class scene;
class asset_manager;

class material {
public:
  material(asset_handle shader_handle, gl_shader &shader_program);

  std::map<std::string, gl_shader::uniform_type> m_uniforms;
  std::map<std::string, std::any> m_uniform_values;

  template <typename _Ty>
  bool set_uniform_value(const std::string &name, const _Ty &val) {
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
    if (m_uniforms.find(name) == m_uniforms.end()) {
      return false;
    }
#endif
    m_uniform_values[name] = val;
    return true;
  }

  bool set_sampler(const std::string &sampler_name, GLenum texture_slot,
                   texture_entry &tex_entry,
                   GLenum texture_target = GL_TEXTURE_2D);

  void bind_material_uniforms(asset_manager &am);

  gl_shader &m_prog;
  const asset_handle m_shader_handle;
};

class material_sys : public ecs_system {
public:
  material_sys() : ecs_system(hash_utils::get_type_hash<material_sys>()) {}
  void init() override;
  void update(scene &current_scene) override;
  void cleanup() override;
  nlohmann::json serialize(scene &current_scene) override;
  void deserialize(scene &current_scene, nlohmann::json &sys_json) override;

  ~material_sys() {}
};
} // namespace gem