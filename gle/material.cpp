#include "material.h"
#include "material.h"
#include "material.h"
#include "material.h"

struct sampler_info_internal
{
    GLenum      sampler_slot;
    GLenum      texture_target;
    gl_handle   texture_handle;
};

material::material(shader& shader_program)
{
    int uniform_count;
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 16; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length
    glGetProgramiv(shader_program.m_shader_id, GL_ACTIVE_UNIFORMS, &uniform_count);

    for (int i = 0; i < uniform_count; i++)
    {
        glGetActiveUniform(shader_program.m_shader_id, (GLuint)i, bufSize, &length, &size, &type, name);
        std::string uname = std::string(name);
        shader::uniform_type utype = shader::get_type_from_gl(type);
        m_uniforms.emplace(uname, utype);
    }
}

bool material::set_sampler(const std::string& sampler_name, GLenum texture_slot, texture& tex, GLenum texture_target)
{
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
    if (m_uniforms.find(sampler_name) == m_uniforms.end())
    {
        return false;
    }
#endif

    p_uniform_values[sampler_name] = sampler_info_internal{ texture_slot, texture_target, tex.m_handle };
}

void material::bind_material_uniforms(shader& prog)
{
    for (auto& [name, val] : p_uniform_values)
    {
#ifdef ENABLE_MATERIAL_UNIFORM_CHECKS
        if (m_uniforms.find(name) == m_uniforms.end())
        {
            continue;
        }
#endif
        switch (m_uniforms[name])
        {
            // TODO: Image attachments for compute shaders....
            case shader::uniform_type::sampler2D:
            case shader::uniform_type::sampler3D:
            {
                sampler_info_internal& info = std::any_cast<sampler_info_internal>(p_uniform_values[name]);
                texture::bind_sampler_handle(info.texture_handle, info.sampler_slot, info.texture_target);
                break;
            }
            case shader::uniform_type::_int:
            {
                int iv = std::any_cast<int>(p_uniform_values[name]);
                prog.set_int(name, iv);
                break;
            }
            case shader::uniform_type::_float: {
                float fv = std::any_cast<float>(p_uniform_values[name]);
                prog.set_float(name, fv);
                break;
            }
            case shader::uniform_type::vec2: {
                glm::vec2& v2 = std::any_cast<glm::vec2>(p_uniform_values[name]);
                prog.set_vec2(name, v2);
                break;
            }
            case shader::uniform_type::vec3:
            {
                glm::vec3& v3 = std::any_cast<glm::vec3>(p_uniform_values[name]);
                prog.set_vec3(name, v3);
                break;
            }
            case shader::uniform_type::vec4:
            {
                glm::vec4& v4 = std::any_cast<glm::vec4>(p_uniform_values[name]);
                prog.set_vec4(name, v4);
                break;
            }
            case shader::uniform_type::mat3:
            {
                glm::mat3& m3 = std::any_cast<glm::mat3>(p_uniform_values[name]);
                prog.set_mat3(name, m3);
                break;
            }
            case shader::uniform_type::mat4:
            {
                glm::mat4& m4 = std::any_cast<glm::mat4>(p_uniform_values[name]);
                prog.set_mat4(name, m4);
                break;
            }
        }
    }
}
