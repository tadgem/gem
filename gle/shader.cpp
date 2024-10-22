#include "shader.h"
#include "gtc/type_ptr.hpp"
#include <array>
#include <iostream>
#include "utils.h"
#include <sstream>
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

void shader::release()
{
	glDeleteProgram(m_shader_id);
}

void shader::set_bool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), (int)value);
}

void shader::set_int(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void shader::set_uint(const std::string& name, unsigned int value) const
{
	glUniform1ui(glGetUniformLocation(m_shader_id, name.c_str()), value);
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

std::unordered_map<shader::stage, std::string> shader::split_composite_shader(const std::string& input)
{
	static std::unordered_map<std::string, shader::stage> s_known_stages = { {"#frag", shader::stage::fragment}, {"#vert", shader::stage::vertex} , {"#geom", shader::stage::geometry}, {"#comp", shader::stage::compute} };
	//static std::unordered_map<std::string, shader::stage> s_known_stages = { };
	auto stages = std::unordered_map<shader::stage, std::string>();
	std::string version = "";

	std::stringstream input_stream(input);
	std::stringstream stage_stream{};
	std::string line = "";
	std::string stage = "";

	while (std::getline(input_stream, line)) {
		if (line.find("#version") != std::string::npos)
		{
			version = line;
			stage_stream << version << "\n";
			continue;
		}

		for (auto& [known , stage_enum]: s_known_stages)
		{
			if (line.find(known) != std::string::npos)
			{
				if (!stage.empty())
				{
					stages.emplace(shader::stage(s_known_stages[stage]), std::string(stage_stream.str()));
					stage_stream.str(std::string());
					stage_stream.clear();
					stage_stream << version << "\n";
				}

				stage = known;
				break;
			}
		}

		if (line == stage)
		{
			continue;
		}

		stage_stream << line << "\n";
	}

	std::string last_stream = stage_stream.str();
	if (!stage.empty() && !last_stream.empty())
	{
		stages.emplace(s_known_stages[stage], last_stream);
	}

	return stages;
}


shader::uniform_type shader::get_type_from_gl(GLenum type)
{
	switch (type)
	{
		case GL_SAMPLER_2D:
			return uniform_type::sampler2D;
		case GL_SAMPLER_3D:
			return uniform_type::sampler3D;
		case GL_IMAGE_2D:
			return uniform_type::image2D;
		case GL_IMAGE_3D:
			return uniform_type::image3D;
		case GL_INT:
			return uniform_type::_int;
		case GL_FLOAT:
			return uniform_type::_float;
		case GL_FLOAT_VEC2:
			return uniform_type::vec2;
		case GL_FLOAT_VEC3:
			return uniform_type::vec3;
		case GL_FLOAT_VEC4:
			return uniform_type::vec4;
		case GL_FLOAT_MAT3:
			return uniform_type::mat3;
		case GL_FLOAT_MAT4:
			return uniform_type::mat4;
		default:
			return uniform_type::UNKNOWN;
	}

	return uniform_type::UNKNOWN;
}
