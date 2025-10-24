#define GLM_ENABLE_EXPERIMENTAL

#include "gem/material.h"
#include "gem/asset_definitions.h"
#include "gem/asset_manager.h"
#include "gem/engine.h"
#include "gem/profile.h"
#include "gem/scene.h"

namespace gem {

Material::Material(AssetHandle shader_handle, GLShader &program)
    : m_prog(program), m_shader_handle(shader_handle) {
  ZoneScoped;
  int uniform_count;
  GLint size;  // size of the variable
  GLenum type; // type of the variable (float, vec3 or mat4, etc)

  const GLsizei bufSize = 16; // maximum name length
  GLchar name[bufSize];       // variable name in GLSL
  GLsizei length;             // name length
  glGetProgramiv(m_prog.m_shader_id, GL_ACTIVE_UNIFORMS, &uniform_count);

  for (int i = 0; i < uniform_count; i++) {
    glGetActiveUniform(m_prog.m_shader_id, (GLuint)i, bufSize, &length, &size,
                       &type, name);
    std::string uname = std::string(name);
    GLShader::UniformType utype = GLShader::GetUniformTypeFromGL(type);
    m_uniforms.emplace(uname, utype);
  }
}

bool Material::set_sampler(const std::string &sampler_name, GLenum texture_slot,
                           TextureEntry &tex_entry, GLenum texture_target) {
  ZoneScoped;
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
  if (m_uniforms.find(sampler_name) == m_uniforms.end()) {
    return false;
  }
#endif

  m_uniform_values[sampler_name] =
      SamplerInfo{texture_slot, texture_target, tex_entry};

  return true;
}

void Material::bind_material_uniforms(AssetManager &am) {
  ZoneScoped;
  m_prog.Use();
  for (auto &[name, val] : m_uniform_values) {
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
    if (m_uniforms.find(name) == m_uniforms.end()) {
      continue;
    }
#endif
    switch (m_uniforms[name]) {
      // TODO: Image attachments for compute shaders....
    case GLShader::UniformType::sampler2D:
    case GLShader::UniformType::sampler3D: {
      SamplerInfo info = std::any_cast<SamplerInfo>(m_uniform_values[name]);
      if (info.tex_entry.m_texture == nullptr) {
        TextureAsset *ta =
            am.get_asset<Texture, AssetType::texture>(info.tex_entry.m_handle);
        if (!ta) {
          continue;
        }
        if (ta->m_data.m_handle != INVALID_GL_HANDLE) {
          info.tex_entry.m_texture = &ta->m_data;
        }
        m_uniform_values[name] = info;
      }
      int loc = info.sampler_slot - GL_TEXTURE0;
      m_prog.SetInt(name, loc);
      Texture::bind_sampler_handle(info.tex_entry.m_texture->m_handle,
                                   info.sampler_slot);
      break;
    }
    case GLShader::UniformType::_int: {
      int iv = std::any_cast<int>(m_uniform_values[name]);
      m_prog.SetInt(name, iv);
      break;
    }
    case GLShader::UniformType::_float: {
      float fv = std::any_cast<float>(m_uniform_values[name]);
      m_prog.SetFloat(name, fv);
      break;
    }
    case GLShader::UniformType::vec2: {
      glm::vec2 v2 = std::any_cast<glm::vec2>(m_uniform_values[name]);
      m_prog.SetVec2f(name, v2);
      break;
    }
    case GLShader::UniformType::vec3: {
      glm::vec3 v3 = std::any_cast<glm::vec3>(m_uniform_values[name]);
      m_prog.SetVec3f(name, v3);
      break;
    }
    case GLShader::UniformType::vec4: {
      glm::vec4 v4 = std::any_cast<glm::vec4>(m_uniform_values[name]);
      m_prog.SetVec4f(name, v4);
      break;
    }
    case GLShader::UniformType::mat3: {
      glm::mat3 m3 = std::any_cast<glm::mat3>(m_uniform_values[name]);
      m_prog.SetMat3f(name, m3);
      break;
    }
    case GLShader::UniformType::mat4: {
      glm::mat4 m4 = std::any_cast<glm::mat4>(m_uniform_values[name]);
      m_prog.SetMat4f(name, m4);
      break;
    }
    default:
      break;
    }
  }
}

void MaterialSystem::init() { ZoneScoped; }

void MaterialSystem::cleanup() { ZoneScoped; }

void MaterialSystem::update(Scene &current_scene) { ZoneScoped; }

nlohmann::json MaterialSystem::serialize(Scene &current_scene) {
  ZoneScoped;

  nlohmann::json sys_json;

  auto sys_view = current_scene.m_registry.view<Material>();

  for (auto [e, mat] : sys_view.each()) {
    nlohmann::json comp_json;
    comp_json["shader"] = mat.m_shader_handle;
    comp_json["uniforms"] = nlohmann::json();
    for (auto [name, uniform_type] : mat.m_uniforms) {
      if (mat.m_uniform_values.find(name) != mat.m_uniform_values.end()) {
        nlohmann::json uniform_json{};
        uniform_json["uniform_type"] = uniform_type;
        switch (uniform_type) {
        case GLShader::UniformType::sampler2D:
        case GLShader::UniformType::sampler3D: {
          SamplerInfo info =
              std::any_cast<SamplerInfo>(mat.m_uniform_values[name]);
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
    sys_json[get_entity_string(e)] = comp_json;
  }

  return sys_json;
}

void MaterialSystem::deserialize(Scene &current_scene, nlohmann::json &sys_json) {
  ZoneScoped;

  for (auto [entity, entry] : sys_json.items()) {
    entt::entity e = get_entity_from_string(entity);
    e = current_scene.m_registry.create(e);
    AssetHandle shader_handle = entry["shader"];
    auto *shader_asset =
        Engine::assets.get_asset<GLShader, AssetType::shader>(shader_handle);
    Material mat(shader_handle, shader_asset->m_data);

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
        mat.set_sampler(uniform_name, uniform_json["slot"], tex_entry,
                        uniform_json["target"]);
        break;
      }
      default: {
        break;
      }
      }
    }
    current_scene.m_registry.emplace<Material>(e, mat);
  }
}
} // namespace gem