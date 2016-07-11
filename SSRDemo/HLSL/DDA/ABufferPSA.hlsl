Texture2D diffuseMap;
Texture2D bumpMap;
Texture2D alphaMap;
SamplerState sam;


//将整个Z Range分层，每层一个RWTexture
//保存相应的Fragment
//RWTexture2DArray<float4> normalBuckets;
//RWTexture2DArray<float4> colorBuckets;
cbuffer cbMaterial
{
    float4 specular;
};
cbuffer cbView
{
    float4x4 view;
};




struct VertexOut
{
	float4 posH:SV_POSITION;
	float3 posV:POSITION;
	float3 normalV:NORMAL0;
	float3 normalW:NORMAL1;
	float2 tex:TEXCOORD;
	float3 tangentW:TANGENT;
};

struct PixelOut
{
    float4 posV : SV_Target0;
    float4 normal : SV_Target1;
    float4 color : SV_Target2;
    float4 spec : SV_Target3;
};

PixelOut main(VertexOut pin)
{
    //透明性剔除
    clip(alphaMap.Sample(sam, pin.tex) - 0.2f);

    PixelOut pout;
    pout.posV = float4(pin.posV, 1.f);
    pout.normal = float4(pin.normalV, pin.posV.z);
    const float3 lightDir = normalize(float3(1, 1, 1));
    float factor = max(dot(lightDir, pin.normalW), 0.f);
    pout.color = diffuseMap.Sample(sam, pin.tex) *
		(float4(0.2f, 0.2f, 0.2f, 0.f) + factor * float4(1.f, 1.f, 1.f, 0.f));
    pout.spec = specular;
    //计算Bucket
//    uint b = max(0.f, (uint) log2(pin.posV.z - nearZ) - 1);
//    normalBuckets[uint3(pin.posH.xy, b)] = pout.normal;
//    colorBuckets[uint3(pin.posH.xy, b)] = pout.color;

    return pout;
}
