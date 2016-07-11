//´¹Ö±Blur
//Blur the temporal map horizontally  to lower mip map level 
Texture2D colorMap;
SamplerState samPoint;
struct VertexOut
{
    float4 posH : SV_Position;
    float2 tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_Target
{
    float4 color = 0.f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(-3, 0)) * 0.001f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(-2, 0)) * 0.0028f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(-1, 0)) * 0.233f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(0, 0)) * 0.474f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(1, 0)) * 0.233f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(2, 0)) * 0.028f;
    color += colorMap.SampleLevel(samPoint,
    pin.tex, 0.f, int2(3, 0)) * 0.001f;

    return color;
}