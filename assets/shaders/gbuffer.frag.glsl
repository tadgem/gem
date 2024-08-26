#version 450


layout(location = 0) in vec2 aUV;
layout(location = 1) in vec3 aPosition;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec2 oDiffuse;
layout(location = 1) out vec3 oPosition;
layout(location = 2) out vec3 oNormal;

out vec4 FragColor;

uniform sampler2D uDiffuseSampler;


void main()
{
	oDiffuse = texture(uDiffuseSampler, aUV);
	oPosition = aPosition;
	oNormal = aNormal;
}