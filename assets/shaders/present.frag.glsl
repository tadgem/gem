#version 450

out vec4 FragColor;

layout (location = 0) in vec2 aUV;

uniform sampler2D u_image_sampler;

void main()
{
   FragColor = texture(u_image_sampler, aUV);
}