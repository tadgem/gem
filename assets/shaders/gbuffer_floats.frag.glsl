#version 450


layout(location = 0) in vec2 aUV;
layout(location = 1) in vec3 aPosition;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec3 oDiffuse;
layout(location = 1) out vec3 oPosition;
layout(location = 2) out vec3 oNormal;
layout(location = 3) out vec3 oPBR;

uniform float u_diffuse;
uniform float u_metallic;
uniform float u_roughness;
uniform float u_ao;



void main()
{
    oDiffuse = pow(texture(u_diffuse_map, aUV).rgb, vec3(2.2));
	oPosition = aPosition;
    oNormal = aNormal;
    oPBR = vec3(u_metallic, u_roughness, u_ao);
}