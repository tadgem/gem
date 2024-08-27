#pragma once
#include <string>
#include "GL/glew.h"
#include "glm.hpp"

class shader
{
public:
	unsigned int m_shader_id;

	shader(const std::string& vert, const std::string& frag);
    shader(const std::string& vert, const std::string& geom, const std::string& frag);

    void use();
    
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, glm::vec2 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setVec4(const std::string& name, glm::vec4 value) const;
    void setMat3(const std::string& name, glm::mat3 value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;


    static int compile_shader(const std::string& source, GLenum shader_stage);
    static int link_shader(unsigned int vert, unsigned int frag);
};