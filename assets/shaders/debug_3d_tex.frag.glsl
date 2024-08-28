#version 450

out vec4 FragColor;

layout(location = 0) in vec3 oUVW;

uniform sampler3D u_volume;

void main()
{
	FragColor = texture(u_volume, oUVW);
}