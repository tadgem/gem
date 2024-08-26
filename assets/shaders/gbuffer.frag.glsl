#version 450


layout(location = 0) in vec2 aUV;
layout(location = 1) in vec3 aPosition;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec3 oDiffuse;
layout(location = 1) out vec3 oPosition;
layout(location = 2) out vec3 oNormal;

out vec4 FragColor;

uniform sampler2D u_diffuse_sampler;


void main()
{
	oDiffuse = texture(u_diffuse_sampler, aUV).xyz;
	oPosition = aPosition;
	oNormal = aNormal;
}