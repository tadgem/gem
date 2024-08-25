#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_mvp;

layout(location = 0) out vec2 oUV;

void main()
{
    oUV = aUV;
    gl_Position = u_mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}