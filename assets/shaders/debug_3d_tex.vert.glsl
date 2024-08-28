#version 450

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 oUVW;

uniform mat4 u_mvp;

void main()
{
	int z = gl_InstanceID % 128;
	int y = (gl_InstanceID / 128) % 128;
	int x = gl_InstanceID / (128 * 128);
	
	oUVW = vec3(x, y, z);

	gl_Position = u_mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}