#vert

struct VertexInput
{
    float3 pos : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

struct VertexOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VertexOutput main(VertexInput input)
{
    VertexOutput o;
    o.uv = input.uv;
    o.pos = float4(input.pos, 0.0);
    return o;
}

#frag
struct FragInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};


Texture2D<float4> Texture : register(t0, space1);
SamplerState TextureSampler : register(s0, space1);


float4 main(FragInput input) : SV_Target0
{
    return Texture.Sample(TextureSampler, input.uv);
}