struct VertexIn
{
    float3 posH : POSITION;
    float3 tex : TEXCOORD;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float2 tex : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    vout.posH = float4(vin.posH, 1.f);
    vout.tex = vin.tex.xy;
    return vout;
}