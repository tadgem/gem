#pragma once
#include "shader.h"

class material
{
public:
	material(shader& shader_program);

	static shader::uniform_type get_type_from_gl(GLenum type);
};