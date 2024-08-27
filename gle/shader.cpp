#include "shader.h"
#include "shader.h"
#include "gtc/type_ptr.hpp"

#include <iostream>


shader::shader(const std::string& vert, const std::string& frag)
{
	auto v = compile_shader(vert, GL_VERTEX_SHADER);
	auto f = compile_shader(frag, GL_FRAGMENT_SHADER);

	m_shader_id = link_shader(v, f);

	glDeleteShader(v);
	glDeleteShader(f);

}

void shader::use()
{
	glUseProgram(m_shader_id);
}

void shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), (int)value);
}

void shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void shader::setVec2(const std::string& name, glm::vec2 value) const
{
	glUniform2f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y);
}

void shader::setVec3(const std::string& name, glm::vec3 value) const
{
	glUniform3f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y, value.z);
}

void shader::setVec4(const std::string& name, glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(m_shader_id, name.c_str()), value.x, value.y, value.z, value.w);
}

void shader::setMat3(const std::string& name, glm::mat3 value) const
{
	glUniformMatrix3fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void shader::setMat4(const std::string& name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

int shader::compile_shader(const std::string& source, GLenum shader_stage)
{
	const char* src = source.c_str();
	auto s = glCreateShader(shader_stage);
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

int shader::link_shader(unsigned int vert, unsigned int frag)
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
