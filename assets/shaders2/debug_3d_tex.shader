#version 450
#vert
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aUV;
layout(location = 2) in mat4 iTransform;

layout(location = 0) out vec3 oUVW;

uniform mat4 viewProjection;

void main()
{	
	oUVW = aUV;
	vec3 worldPos = vec3(iTransform * vec4(aPos, 1.0));
	gl_Position = viewProjection * vec4(worldPos, 1.0);
}

#frag 
out vec4 FragColor;

layout(location = 0) in vec3 oUVW;

uniform sampler3D u_volume;

void main()
{
	vec4 col = texture(u_volume, oUVW);
	if (col.w < 0.1)
	{
		discard;
	}
	FragColor = texture(u_volume, oUVW) * 2.0f;
}