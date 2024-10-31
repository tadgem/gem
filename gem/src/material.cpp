#define GLM_ENABLE_EXPERIMENTAL

#include "gem/material.h"
#include "gem/asset_definitions.h"
#include "gem/asset_manager.h"
#include "gem/engine.h"
#include "gem/profile.h"
#include "gem/scene.h"

namespace gem {

material::material(asset_handle shader_handle, shader &program)
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
    shader::uniform_type utype = shader::get_type_from_gl(type);
    m_uniforms.emplace(uname, utype);
  }
}

bool material::set_sampler(const std::string &sampler_name, GLenum texture_slot,
                           texture_entry &tex_entry, GLenum texture_target) {
  ZoneScoped;
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
  if (m_uniforms.find(sampler_name) == m_uniforms.end()) {
    return false;
  }
#endif

  m_uniform_values[sampler_name] =
      sampler_info{texture_slot, texture_target, tex_entry};

  return true;
}

void material::bind_material_uniforms(asset_manager &am) {
  ZoneScoped;
  m_prog.use();
  for (auto &[name, val] : m_uniform_values) {
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
    if (m_uniforms.find(name) == m_uniforms.end()) {
      continue;
    }
#endif
    switch (m_uniforms[name]) {
      // TODO: Image attachments for compute shaders....
    case shader::uniform_type::sampler2D:
    case shader::uniform_type::sampler3D: {
      sampler_info info = std::any_cast<sampler_info>(m_uniform_values[name]);
      if (info.tex_entry.m_texture == nullptr) {
        texture_asset *ta =
            am.get_asset<texture, asset_type::texture>(info.tex_entry.m_handle);
        if (!ta) {
          continue;
        }
        if (ta->m_data.m_handle != INVALID_GL_HANDLE) {
          info.tex_entry.m_texture = &ta->m_data;
        }
        m_uniform_values[name] = info;
      }
      int loc = info.sampler_slot - GL_TEXTURE0;
      m_prog.set_int(name, loc);
      texture::bind_sampler_handle(info.tex_entry.m_texture->m_handle,
                                   info.sampler_slot);
      break;
    }
    case shader::uniform_type::_int: {
      int iv = std::any_cast<int>(m_uniform_values[name]);
      m_prog.set_int(name, iv);
      break;
    }
    case shader::uniform_type::_float: {
      float fv = std::any_cast<float>(m_uniform_values[name]);
      m_prog.set_float(name, fv);
      break;
    }
    case shader::uniform_type::vec2: {
      glm::vec2 v2 = std::any_cast<glm::vec2>(m_uniform_values[name]);
      m_prog.set_vec2(name, v2);
      break;
    }
    case shader::uniform_type::vec3: {
      glm::vec3 v3 = std::any_cast<glm::vec3>(m_uniform_values[name]);
      m_prog.set_vec3(name, v3);
      break;
    }
    case shader::uniform_type::vec4: {
      glm::vec4 v4 = std::any_cast<glm::vec4>(m_uniform_values[name]);
      m_prog.set_vec4(name, v4);
      break;
    }
    case shader::uniform_type::mat3: {
      glm::mat3 m3 = std::any_cast<glm::mat3>(m_uniform_values[name]);
      m_prog.set_mat3(name, m3);
      break;
    }
    case shader::uniform_type::mat4: {
      glm::mat4 m4 = std::any_cast<glm::mat4>(m_uniform_values[name]);
      m_prog.set_mat4(name, m4);
      break;
    }
    default:
      break;
    }
  }
}

void material_sys::init() { ZoneScoped; }

void material_sys::cleanup() { ZoneScoped; }

void material_sys::update(scene &current_scene) { ZoneScoped; }

nlohmann::json material_sys::serialize(scene &current_scene) {
  ZoneScoped;

  nlohmann::json sys_json;

  auto sys_view = current_scene.m_registry.view<material>();

  for (auto [e, mat] : sys_view.each()) {
    nlohmann::json comp_json;
    comp_json["shader"] = mat.m_shader_handle;
    comp_json["uniforms"] = nlohmann::json();
    for (auto [name, uniform_type] : mat.m_uniforms) {
      if (mat.m_uniform_values.find(name) != mat.m_uniform_values.end()) {
        nlohmann::json uniform_json{};
        uniform_json["uniform_type"] = uniform_type;
        switch (uniform_type) {
        case shader::uniform_type::sampler2D:
        case shader::uniform_type::sampler3D: {
          sampler_info info =
              std::any_cast<sampler_info>(mat.m_uniform_values[name]);
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

void material_sys::deserialize(scene &current_scene, nlohmann::json &sys_json) {
  ZoneScoped;

  for (auto [entity, entry] : sys_json.items()) {
    entt::entity e = get_entity_from_string(entity);
    e = current_scene.m_registry.create(e);
    asset_handle shader_handle = entry["shader"];
    auto *shader_asset =
        engine::assets.get_asset<shader, asset_type::shader>(shader_handle);
    material mat(shader_handle, shader_asset->m_data);

    nlohmann::json uniforms = entry["uniforms"];

    spdlog::info("entity : {} : material json : {}", entity, entry.dump());

    for (auto &[uniform_name, uniform_json] : uniforms.items()) {
      std::string uniform_json_str = uniform_json.dump();

      spdlog::info("entity : {} : uniform json {}", entity, uniform_json_str);
      shader::uniform_type uniform_type = uniform_json["uniform_type"];

      switch (uniform_type) {
      case shader::uniform_type::sampler2D:
      case shader::uniform_type::sampler3D: {
        texture_entry tex_entry = uniform_json["entry"];
        mat.set_sampler(uniform_name, uniform_json["slot"], tex_entry,
                        uniform_json["target"]);
        break;
      }
      default: {
        break;
      }
      }
    }
    current_scene.m_registry.emplace<material>(e, mat);
  }
}
} // namespace gem