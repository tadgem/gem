#define GLM_ENABLE_EXPERIMENTAL

#include "gem/material.h"
#include "gem/asset_definitions.h"
#include "gem/asset_manager.h"
#include "gem/engine.h"
#include "gem/profile.h"
#include "gem/scene.h"

namespace gem {

Material::Material(AssetHandle shader_handle, GLShader &program)
    : program(program), shader_handle(shader_handle) {
  ZoneScoped;
  int uniform_count;
  GLint size;  // size of the variable
  GLenum type; // type of the variable (float, vec3 or mat4, etc)

  const GLsizei bufSize = 16; // maximum name length
  GLchar name[bufSize];       // variable name in GLSL
  GLsizei length;             // name length
  glGetProgramiv(program.m_shader_id, GL_ACTIVE_UNIFORMS, &uniform_count);

  for (int i = 0; i < uniform_count; i++) {
    glGetActiveUniform(program.m_shader_id, (GLuint)i, bufSize, &length, &size,
                       &type, name);
    std::string uname = std::string(name);
    GLShader::UniformType utype = GLShader::GetUniformTypeFromGL(type);
    uniforms.emplace(uname, utype);
  }
}

bool Material::SetSampler(const std::string &sampler_name, GLenum texture_slot,
                           TextureEntry &tex_entry, GLenum texture_target) {
  ZoneScoped;
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
  if (uniforms.find(sampler_name) == uniforms.end()) {
    return false;
  }
#endif

  uniform_values[sampler_name] =
      SamplerInfo{texture_slot, texture_target, tex_entry};

  return true;
}

void Material::BindUniforms(AssetManager &am) {
  ZoneScoped;
  program.Use();
  for (auto &[name, val] : uniform_values) {
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
    if (uniforms.find(name) == uniforms.end()) {
      continue;
    }
#endif
    switch (uniforms[name]) {
      // TODO: Image attachments for compute shaders....
    case GLShader::UniformType::sampler2D:
    case GLShader::UniformType::sampler3D: {
      SamplerInfo info = std::any_cast<SamplerInfo>(uniform_values[name]);
      if (info.tex_entry.texture_data == nullptr) {
        TextureAsset *ta =
            am.GetAsset<Texture, AssetType::texture>(info.tex_entry.handle);
        if (!ta) {
          continue;
        }
        if (ta->data.handle != INVALID_GL_HANDLE) {
          info.tex_entry.texture_data = &ta->data;
        }
        uniform_values[name] = info;
      }
      int loc = info.sampler_slot - GL_TEXTURE0;
      program.SetInt(name, loc);
      Texture::BindSamplerHandle(info.tex_entry.texture_data->handle,
                                   info.sampler_slot);
      break;
    }
    case GLShader::UniformType::_int: {
      int iv = std::any_cast<int>(uniform_values[name]);
      program.SetInt(name, iv);
      break;
    }
    case GLShader::UniformType::_float: {
      float fv = std::any_cast<float>(uniform_values[name]);
      program.SetFloat(name, fv);
      break;
    }
    case GLShader::UniformType::vec2: {
      glm::vec2 v2 = std::any_cast<glm::vec2>(uniform_values[name]);
      program.SetVec2f(name, v2);
      break;
    }
    case GLShader::UniformType::vec3: {
      glm::vec3 v3 = std::any_cast<glm::vec3>(uniform_values[name]);
      program.SetVec3f(name, v3);
      break;
    }
    case GLShader::UniformType::vec4: {
      glm::vec4 v4 = std::any_cast<glm::vec4>(uniform_values[name]);
      program.SetVec4f(name, v4);
      break;
    }
    case GLShader::UniformType::mat3: {
      glm::mat3 m3 = std::any_cast<glm::mat3>(uniform_values[name]);
      program.SetMat3f(name, m3);
      break;
    }
    case GLShader::UniformType::mat4: {
      glm::mat4 m4 = std::any_cast<glm::mat4>(uniform_values[name]);
      program.SetMat4f(name, m4);
      break;
    }
    default:
      break;
    }
  }
}

void MaterialSystem::Init() { ZoneScoped; }

void MaterialSystem::Cleanup() { ZoneScoped; }

void MaterialSystem::Update(Scene &current_scene) { ZoneScoped; }

nlohmann::json MaterialSystem::Serialize(Scene &current_scene) {
  ZoneScoped;

  nlohmann::json sys_json;

  auto sys_view = current_scene.registry.view<Material>();

  for (auto [e, mat] : sys_view.each()) {
    nlohmann::json comp_json;
    comp_json["shader"] = mat.shader_handle;
    comp_json["uniforms"] = nlohmann::json();
    for (auto [name, uniform_type] : mat.uniforms) {
      if (mat.uniform_values.find(name) != mat.uniform_values.end()) {
        nlohmann::json uniform_json{};
        uniform_json["uniform_type"] = uniform_type;
        switch (uniform_type) {
        case GLShader::UniformType::sampler2D:
        case GLShader::UniformType::sampler3D: {
          SamplerInfo info =
              std::any_cast<SamplerInfo>(mat.uniform_values[name]);
          uniform_json["slot"] = info.sampler_slot;
          uniform_json["target"] = info.texture_target;
          uniform_json["entry"] = info.tex_entry;
          break;
        }
        default:
          break;
        }
        comp_json["uniforms"][name] = uniform_json;
      }
    }
    sys_json[GetEntityIDString(e)] = comp_json;
  }

  return sys_json;
}

void MaterialSystem::Deserialize(Scene &current_scene, nlohmann::json &sys_json) {
  ZoneScoped;

  for (auto [entity, entry] : sys_json.items()) {
    entt::entity e = GetEntityIDFromString(entity);
    e = current_scene.registry.create(e);
    AssetHandle shader_handle = entry["shader"];
    auto *shader_asset =
        Engine::assets.GetAsset<GLShader, AssetType::shader>(shader_handle);
    Material mat(shader_handle, shader_asset->data);

    nlohmann::json uniforms = entry["uniforms"];

    spdlog::info("entity : {} : material json : {}", entity, entry.dump());

    for (auto &[uniform_name, uniform_json] : uniforms.items()) {
      std::string uniform_json_str = uniform_json.dump();

      spdlog::info("entity : {} : uniform json {}", entity, uniform_json_str);
      GLShader::UniformType uniform_type = uniform_json["uniform_type"];

      switch (uniform_type) {
      case GLShader::UniformType::sampler2D:
      case GLShader::UniformType::sampler3D: {
        TextureEntry tex_entry = uniform_json["entry"];
        mat.SetSampler(uniform_name, uniform_json["slot"], tex_entry,
                        uniform_json["target"]);
        break;
      }
      default: {
        break;
      }
      }
    }
    current_scene.registry.emplace<Material>(e, mat);
  }
}
} // namespace gem