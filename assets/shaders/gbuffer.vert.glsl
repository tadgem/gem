#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

layout(location = 0) out vec2 oUV;
layout(location = 1) out vec3 oPosition;
layout(location = 2) out vec3 oNormal;
layout(location = 3) out vec2 oVelocity;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_last_vp;
uniform mat4 u_normal;
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

void main()
{
    oUV = aUV;
    oNormal = (vec4(aNormal, 1.0) * u_normal).xyz;
    oPosition = (vec4(aPos , 1.0) * u_model).xyz;

    int jitter_index = u_frame_index % 16;
    vec2 offset = halton_seq[jitter_index];

    offset.x = ((offset.x-0.5) / 1280.0) * 2.0;
    offset.y = ((offset.y-0.5) / 720.0 ) * 2.0;

    vec4 pos =  u_mvp * vec4(aPos, 1.0);
    pos += vec4(offset * pos.z, 0.0, 0.0);

    vec2 last_offset = halton_seq[max(jitter_index - 1, 0)];
    vec4 last_pos = u_last_vp * u_model * vec4(aPos, 1.0);
    last_pos += vec4(last_offset * last_pos.z, 0.0, 0.0);
    oVelocity = (pos - last_pos).xy;

    gl_Position = pos;
}