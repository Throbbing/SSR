//´¹Ö±Blur
//The blur is applied vertically to a temporal map
Texture2D colorMap;
SamplerState samPoint;


cbuffer cbBlur:register(b0)
{
    int curLevel;
};
struct VertexOut
{
    float4 posH : SV_Position;
    float2 tex : TEXCOORD;
};

float4 main(VertexOut pin):SV_Target
{
    float4 color = 0.f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, -3)) * 0.001f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, -2)) * 0.0028f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, -1)) * 0.233f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, 0)) * 0.474f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, 1)) * 0.233f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, 2)) * 0.028f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, curLevel, int2(0, 3)) * 0.001f;

    return color;
}