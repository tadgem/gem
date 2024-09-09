#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aUV;
layout(location = 2) in mat4 iTransform;

layout(location = 0) out vec3 oUVW;

uniform mat4 viewProjection;

void main()
{
	int z = gl_InstanceID % 128;
	int y = (gl_InstanceID / 128) % 128;
	int x = gl_InstanceID / (128 * 128);
	
	oUVW = aUV;

	vec3 worldPos = vec3(iTransform * vec4(aPos, 1.0));
	gl_Position = viewProjection * vec4(worldPos, 1.0);
}