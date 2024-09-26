#include "shader.h"
#include "shader.h"
#include "shader.h"
#include "shader.h"
#include "shader.h"
#include "shader.h"
#include "gtc/type_ptr.hpp"

#include <iostream>


shader::shader(const std::string& comp)
{
	auto c = compile_shader(comp, GL_COMPUTE_SHADER);
	m_shader_id = link_shader(c);

	glDeleteShader(c);
}

shader::shader(const std::string& vert, const std::string& frag)
{
	auto v = compile_shader(vert, GL_VERTEX_SHADER);
	auto f = compile_shader(frag, GL_FRAGMENT_SHADER);

	m_shader_id = link_shader(v, f);

	glDeleteShader(v);
	glDeleteShader(f);

}

shader::shader(const std::string& vert, const std::string& geom, const std::string& frag)
{
	auto v = compile_shader(vert, GL_VERTEX_SHADER);
	auto g = compile_shader(geom, GL_GEOMETRY_SHADER);
	auto f = compile_shader(frag, GL_FRAGMENT_SHADER);

	m_shader_id = link_shader(v, g, f);

	glDeleteShader(v);
	glDeleteShader(g);
	glDeleteShader(f);

}

void shader::use()
{
	glUseProgram(m_shader_id);
}

void shader::set_bool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), (int)value);
}

void shader::set_int(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void shader::set_float(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void shader::set_vec2(const std::string& name, glm::vec2 value) const
{
	glUniform2f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y);
}

void shader::set_vec3(const std::string& name, glm::vec3 value) const
{
	glUniform3f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y, value.z);
}

void shader::set_vec4(const std::string& name, glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y, value.z, value.w);
}

void shader::set_ivec2(const std::string& name, glm::ivec2 value) const
{
	glUniform2i(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y);
}

void shader::set_ivec3(const std::string& name, glm::ivec3 value) const
{
	glUniform3i(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y, value.z);
}

void shader::set_ivec4(const std::string& name, glm::ivec4 value) const
{
	glUniform4i(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y, value.z, value.w);
}

void shader::set_mat3(const std::string& name, glm::mat3 value) const
{
	glUniformMatrix3fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void shader::set_mat4(const std::string& name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

gl_handle shader::compile_shader(const std::string& source, GLenum shader_stage)
{
	const char* src = source.c_str();
	gl_handle s = glCreateShader(shader_stage);
	glShaderSource(s, 1, &src, NULL);
	glCompileShader(s);

	int success = 0;

	glGetShaderiv(s, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(s, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	return s;
}

int shader::link_shader(gl_handle comp)
{
	auto prog_id = glCreateProgram();
	glAttachShader(prog_id, comp);
	glLinkProgram(prog_id);

	int success = 0;

	glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(prog_id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	return prog_id;
}

int shader::link_shader(gl_handle vert, gl_handle frag)
{
	auto prog_id = glCreateProgram();
	glAttachShader(prog_id, vert);
	glAttachShader(prog_id, frag);
	glLinkProgram(prog_id);

	int success = 0;

	glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(prog_id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	return prog_id;
}

int shader::link_shader(gl_handle vert, gl_handle geom, gl_handle frag)
{
	auto prog_id = glCreateProgram();
	glAttachShader(prog_id, vert);
	glAttachShader(prog_id, geom);
	glAttachShader(prog_id, frag);
	glLinkProgram(prog_id);

	int success = 0;

	glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(prog_id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	return prog_id;
}
