#version 450
#vert

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

layout(location = 0) out vec2 oUV;
layout(location = 1) out vec4 oPosition;
layout(location = 2) out vec3 oNormal;
layout(location = 3) out vec4 oClipPos;
layout(location = 4) out vec4 oLastClipPos;

uniform mat4      u_vp;
uniform mat4      u_model;
uniform mat4      u_last_vp;
uniform mat4      u_last_model;
uniform mat4      u_normal;
uniform int       u_frame_index;
uniform vec2      u_resolution;

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

void main()
{
    oUV = aUV;
    oNormal = (vec4(aNormal, 1.0) * u_normal).xyz;
    oPosition = (u_model * vec4(aPos , 1.0));
    vec4 pos =  u_vp * u_model * vec4(aPos, 1.0);
    oClipPos = pos;

    oLastClipPos = u_last_vp * u_last_model * vec4(aPos, 1.0);

    int jitter_index = u_frame_index % 16;
    vec2 offset = halton_seq[jitter_index];
    offset.x = ((offset.x-0.5) / u_resolution.x) * 2.0;
    offset.y = ((offset.y-0.5) / u_resolution.y ) * 2.0;

    pos += vec4(offset * pos.z, 0.0, 0.0);
    gl_Position = pos;
}

#frag

layout(location = 0) in vec2 aUV;
layout(location = 1) in vec4 aPosition;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aClipPos;
layout(location = 4) in vec4 aLastClipPos;


layout(location = 0) out vec3 oDiffuse;
layout(location = 1) out vec4 oPosition;
layout(location = 2) out vec3 oNormal;
layout(location = 3) out vec3 oPBR;
layout(location = 4) out vec2 oVelocity;
layout(location = 5) out vec3 oEntityID;

uniform vec3        u_diffuse_map;
uniform float       u_metallic_map;
uniform float       u_roughness_map;
uniform float       u_ao_map;
uniform sampler2D   u_prev_position_map;

uniform mat4    u_last_vp;
uniform int     u_frame_index;
uniform int     u_entity_index;

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

vec3 UnpackNormalMap( vec3 TextureSample )
{
	vec2 NormalXY = TextureSample.rg;
	
	NormalXY = NormalXY * vec2(2.0,2.0) - vec2(1.0,1.0);
	float NormalZ = sqrt( clamp(( 1.0f - dot( NormalXY, NormalXY ) ),0.0,1.0));
	return vec3( NormalXY.xy, NormalZ);
}

vec3 getNormalFromMap() {
    vec3 tangentNormal = aNormal;

    if(abs(tangentNormal.z) < 0.0001) {
        tangentNormal = UnpackNormalMap(tangentNormal);
    }

    vec3 Q1 = dFdx(aPosition.xyz);
    vec3 Q2 = dFdy(aPosition.xyz);

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
    vec4 inDiffuse = vec4(u_diffuse_map, 1.0);
    if(inDiffuse.w < 0.25)
    {
        discard;
    }

    oDiffuse = pow(inDiffuse.xyz, vec3(2.2));
	oPosition = aPosition;
    oNormal = aNormal;

    float r = ((u_entity_index & 0x000000FF) >>  0);
    float g = ((u_entity_index & 0x0000FF00) >>  8);
    float b = ((u_entity_index & 0x00FF0000) >> 16);

    oEntityID = vec3(r,g,b);

    vec2 currentPosNDC = aClipPos.xy / aClipPos.z;
    vec2 previousPosNDC = aLastClipPos.xy / aLastClipPos.z;

    // velocity 
    oVelocity = currentPosNDC - previousPosNDC;

    float metallic = u_metallic_map;
    float roughness = u_roughness_map;
    float ao = u_ao_map;

    oPBR = vec3(metallic, roughness, ao);
}