#version 450


layout(location = 0) in vec2 aUV;
layout(location = 1) in vec3 aPosition;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec3 oDiffuse;
layout(location = 1) out vec3 oPosition;
layout(location = 2) out vec3 oNormal;
layout(location = 3) out vec3 oPBR;

uniform sampler2D u_diffuse_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_metallic_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_ao_map;


vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_normal_map, aUV).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(aPosition);
    vec3 Q2 = dFdy(aPosition);
    vec2 st1 = dFdx(aUV);
    vec2 st2 = dFdy(aUV);

    vec3 N = normalize(aNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{

    oDiffuse = pow(texture(u_diffuse_map, aUV).rgb, vec3(2.2));
	oPosition = aPosition;
    oNormal = getNormalFromMap();

    float metallic = texture(u_metallic_map, aUV).r;
    float roughness = texture(u_roughness_map, aUV).r;
    float ao = texture(u_ao_map, aUV).r;
    oPBR = vec3(metallic, roughness, ao);
}