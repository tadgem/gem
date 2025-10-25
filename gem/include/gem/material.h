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

  std::map<std::string, GLShader::UniformType> uniforms;
  std::map<std::string, std::any> uniform_values;

  template <typename _Ty>
  bool SetUniformValue(const std::string &name, const _Ty &val) {
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
    if (uniforms.find(name) == uniforms.end()) {
      return false;
    }
#endif
    uniform_values[name] = val;
    return true;
  }

  bool SetSampler(const std::string &sampler_name, GLenum texture_slot,
                   TextureEntry &tex_entry,
                   GLenum texture_target = GL_TEXTURE_2D);

  void BindUniforms(AssetManager &am);

  GLShader &program;
  const AssetHandle shader_handle;
};

class MaterialSystem : public ECSSystem {
public:
  MaterialSystem() : ECSSystem(HashUtils::GetTypeHash<MaterialSystem>()) {}
  void Init() override;
  void Update(Scene &current_scene) override;
  void Cleanup() override;
  nlohmann::json Serialize(Scene &current_scene) override;
  void Deserialize(Scene &current_scene, nlohmann::json &sys_json) override;

  ~MaterialSystem() {}
};
} // namespace gem