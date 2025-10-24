#pragma once
#include "gem/dbg_memory.h"
#include "gem/ecs_system.h"
#include "gem/gl/gl_shader.h"
#include "gem/model.h"
#include "gem/texture.h"
#include <any>
#include <map>
#include <string>
#define ENABLE_MATERIAL_UNIFORM_CHECKS
namespace gem {

class Scene;
class AssetManager;

class Material {
public:
  Material(AssetHandle shader_handle, GLShader &shader_program);

  std::map<std::string, GLShader::UniformType> m_uniforms;
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
                   TextureEntry &tex_entry,
                   GLenum texture_target = GL_TEXTURE_2D);

  void bind_material_uniforms(AssetManager &am);

  GLShader &m_prog;
  const AssetHandle m_shader_handle;
};

class MaterialSystem : public ECSSystem {
public:
  MaterialSystem() : ECSSystem(HashUtils::get_type_hash<MaterialSystem>()) {}
  void init() override;
  void update(Scene &current_scene) override;
  void cleanup() override;
  nlohmann::json serialize(Scene &current_scene) override;
  void deserialize(Scene &current_scene, nlohmann::json &sys_json) override;

  ~MaterialSystem() {}
};
} // namespace gem