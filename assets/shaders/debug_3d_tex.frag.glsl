#version 450

out vec4 FragColor;

layout(location = 0) in vec3 oUVW;

uniform sampler3D u_volume;

void main()
{
	vec4 col = texture(u_volume, oUVW);
	if (col.w < 0.75)
	{
		discard;
	}
	FragColor = texture(u_volume, oUVW) * 2.0f;
}