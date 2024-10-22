#version 450
#vert

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

layout(location = 0) out vec2 oUV;

void main()
{
    oUV = aUV;
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}

#frag

out vec4 FragColor;

layout (location = 0) in vec2 aUV;

uniform sampler2D u_image_sampler;

void main()
{
   FragColor = texture(u_image_sampler, aUV);
}