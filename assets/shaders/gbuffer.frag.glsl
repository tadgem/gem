#version 450


layout(location = 0) in vec2 aUV;
layout(location = 1) in vec3 aPosition;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aClipPos;

layout(location = 0) out vec3 oDiffuse;
layout(location = 1) out vec3 oPosition;
layout(location = 2) out vec3 oNormal;
layout(location = 3) out vec3 oPBR;
layout(location = 4) out vec2 oVelocity;

uniform sampler2D u_diffuse_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_metallic_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_ao_map;
uniform sampler2D u_prev_position_map;

uniform mat4 u_last_vp;
uniform int u_frame_index;

const vec2 halton_seq[16] = vec2[16] 
(
    vec2(0.500000, 0.333333),
    vec2(0.250000, 0.666667),
    vec2(0.750000, 0.111111),
    vec2(0.125000, 0.444444),
    vec2(0.625000, 0.777778),
    vec2(0.375000, 0.222222),
    vec2(0.875000, 0.555556),
    vec2(0.062500, 0.888889),
    vec2(0.562500, 0.037037),
    vec2(0.312500, 0.370370),
    vec2(0.812500, 0.703704),
    vec2(0.187500, 0.148148),
    vec2(0.687500, 0.481481),
    vec2(0.437500, 0.814815),
    vec2(0.937500, 0.259259),
    vec2(0.031250, 0.592593)
);


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

    vec4 lastPixelPos = texture(u_prev_position_map, aUV);
    vec4 lastClipPos = (u_last_vp * lastPixelPos);


    vec2 currentPosNDC = aClipPos.xy / aClipPos.w;
    vec2 previousPosNDC = lastClipPos.xy / lastClipPos.w;

    // velocity 
    oVelocity = currentPosNDC - previousPosNDC;
    float metallic = texture(u_metallic_map, aUV).r;
    float roughness = texture(u_roughness_map, aUV).r;
    float ao = texture(u_ao_map, aUV).r;
    oPBR = vec3(metallic, roughness, ao);
}