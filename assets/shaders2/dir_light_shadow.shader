#version 330
#vert

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}  

#frag
#version 330 core

void main()
{             
    // implicitly
    // gl_FragDepth = gl_FragCoord.z;
}  