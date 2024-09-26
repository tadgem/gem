#include "material.h"

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

        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
    }
}

shader::uniform_type material::get_type_from_gl(GLenum type)
{
    switch (type)
    {
    }
    return shader::uniform_type();
}
