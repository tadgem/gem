#vert

struct VertexInput
{
    float3 pos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
};

struct VertexOutput
{
    float4 pos : SV_Position;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float4 clip_pos : TEXCOORD2;
    float4 last_clip_pos : TEXCOORD3;
};

cbuffer UniformBlock : register(b0, space0)
{
    float4x4 u_vp;
    float4x4 u_model;
    float4x4 u_last_vp;
    float4x4 u_last_model;
    float4x4 u_normal;
    int      u_frame_index;
    float2   u_resolution;
}

static const float2 halton_seq[16] =
{
    float2(0.500000, 0.333333),
    float2(0.250000, 0.666667),
    float2(0.750000, 0.111111),
    float2(0.125000, 0.444444),
    float2(0.625000, 0.777778),
    float2(0.375000, 0.222222),
    float2(0.875000, 0.555556),
    float2(0.062500, 0.888889),
    float2(0.562500, 0.037037),
    float2(0.312500, 0.370370),
    float2(0.812500, 0.703704),
    float2(0.187500, 0.148148),
    float2(0.687500, 0.481481),
    float2(0.437500, 0.814815),
    float2(0.937500, 0.259259),
    float2(0.031250, 0.592593)
};

VertexOutput main(VertexInput input)
{
    VertexOutput o;
    o.normal = mul(u_normal, float4(input.normal, 1.0)).xyz;
    o.uv = input.uv;
    float4 clip_pos = mul(u_vp * u_model, float4(input.pos, 1.0));
    o.clip_pos = clip_pos;
    o.last_clip_pos =  mul(u_last_vp * u_last_model, float4(input.pos, 1.0));

    int jitter_index = u_frame_index % 16;
    float2 offset = halton_seq[jitter_index];
    offset.x = ((offset.x-0.5) / u_resolution.x) * 4.0;
    offset.y = ((offset.y-0.5) / u_resolution.y ) * 4.0;

    clip_pos += float4(offset * input.pos.z, 0.0, 0.0);
    o.pos = mul(u_model, clip_pos);
    return o;
}

#frag
struct FragInput
{
    float4 pos : SV_Position;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float4 clip_pos : TEXCOORD2;
    float4 last_clip_pos : TEXCOORD3;
};

struct FragOutput
{
    float4 diffuse : SV_Target0;
    float4 clip_pos : SV_Target1;
    float4 normal : SV_Target2;
    float4 pbr : SV_Target3;
    float2 velocity : SV_Target4;
    float4 entity_id : SV_Target5;
};

cbuffer UniformBlock : register(b1, space1)
{
    float4x4 u_last_vp;
    int      u_frame_index;
    int      u_entity_index;
}

Texture2D<float4> DiffuseTexture : register(t0, space2);
SamplerState DiffuseSampler : register(s0, space2);
Texture2D<float4> NormalTexture : register(t1, space2);
SamplerState NormalSampler : register(s1, space2);
Texture2D<float4> MetallicTexture : register(t2, space2);
SamplerState MetallicSampler : register(s2, space2);
Texture2D<float4> RoughnessTexture : register(t3, space2);
SamplerState RoughnessSampler : register(s3, space2);
Texture2D<float4> AOTexture : register(t4, space2);
SamplerState AOSampler : register(s4, space2);
Texture2D<float4> PreviousPositionTexture : register(t5, space2);
SamplerState PreviousPositionSampler : register(s5, space2);

static const float2 halton_seq[16] =
{
    float2(0.500000, 0.333333),
    float2(0.250000, 0.666667),
    float2(0.750000, 0.111111),
    float2(0.125000, 0.444444),
    float2(0.625000, 0.777778),
    float2(0.375000, 0.222222),
    float2(0.875000, 0.555556),
    float2(0.062500, 0.888889),
    float2(0.562500, 0.037037),
    float2(0.312500, 0.370370),
    float2(0.812500, 0.703704),
    float2(0.187500, 0.148148),
    float2(0.687500, 0.481481),
    float2(0.437500, 0.814815),
    float2(0.937500, 0.259259),
    float2(0.031250, 0.592593)
};

float3 UnpackNormalMap( float3 TextureSample )
{
    float2 NormalXY = TextureSample.rg;

    NormalXY = NormalXY * float2(2.0,2.0) - float2(1.0,1.0);
    float NormalZ = sqrt( clamp(( 1.0f - dot( NormalXY, NormalXY ) ),0.0,1.0));
    return float3( NormalXY.xy, NormalZ);
}

float3 getNormalFromMap(FragInput input) {
    float3 tangentNormal = NormalTexture.Sample(NormalSampler, input.uv).xyz * 2.0 - 1.0;

    if(abs(tangentNormal.z) < 0.0001) {
        tangentNormal = UnpackNormalMap(tangentNormal);
    }

    float3 Q1 = ddx(input.pos.xyz);
    float3 Q2 = ddy(input.pos.xyz);

    float3 st1 = ddx(float3(input.uv, 0.0));
    float3 st2 = ddy(float3(input.uv, 0.0));

    float3 N = normalize(input.normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(TBN, tangentNormal));
}

FragOutput main(FragInput input)
{
    FragOutput o;
    float4 diffuse = DiffuseTexture.Sample(DiffuseSampler, input.uv);
    o.clip_pos = input.pos;
    o.normal = float4(getNormalFromMap(input), 0.0);

    float r = ((u_entity_index & 0x000000FF) >>  0);
    float g = ((u_entity_index & 0x0000FF00) >>  8);
    float b = ((u_entity_index & 0x00FF0000) >> 16);

    o.entity_id = float4(r,g,b, 0.0);

    float2 currentPosNDC = input.clip_pos.xy / input.clip_pos.z;
    float2 previousPosNDC = input.last_clip_pos.xy / input.last_clip_pos.z;

    // velocity
    o.velocity = currentPosNDC - previousPosNDC;
    float metallic = MetallicTexture.Sample(MetallicSampler, input.uv).r;
    float roughness = RoughnessTexture.Sample(RoughnessSampler, input.uv).r;
    float ao = AOTexture.Sample(AOSampler, input.uv).r;
    o.pbr = float4(metallic, roughness, ao, 0.0);
    return o;
}