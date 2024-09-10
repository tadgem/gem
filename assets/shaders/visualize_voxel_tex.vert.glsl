#version 450 core

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 frag_obj_pos;

uniform mat4 mvp;
uniform mat4 model;


void main()
{
    frag_obj_pos = pos;
    gl_Position = mvp * vec4(pos,1.0);
}