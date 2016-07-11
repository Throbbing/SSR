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

PixelOut main(VertexOut pin )
{
	

    //从Bump Map中获取法线(World Space)
    pin.tangentW = normalize(pin.tangentW);
    float3 Ni = normalize(pin.normalW);
    float3 T = normalize(pin.tangentW - dot(pin.tangentW, Ni) * Ni);
    float3 Bi = normalize(cross(Ni, T));


    float3x3 TBN = float3x3(T, Bi, Ni);

    float3 samNormal = bumpMap.Sample(sam, pin.tex).xyz;
    samNormal = samNormal * 2.f - 1.f;

    float3 normalW = mul(samNormal, TBN);
    normalW = normalize(normalW);

	//将法线变换至Camera Space
    float3 normalV = mul(float4(normalW, 0.f), view).xyz;
    
    PixelOut pout;
    pout.posV = float4(pin.posV, 1.f);
    pout.normal = float4(normalV, pin.posV.z);
    const float3 lightDir = normalize(float3(1, 1, 1));
    float factor = max(dot(lightDir, normalW), 0.f);
    pout.color = diffuseMap.Sample(sam, pin.tex) *
		(float4(0.2f, 0.2f, 0.2f, 0.f) + factor * float4(1.f, 1.f, 1.f, 0.f));
    pout.spec = specular;

    

    


    return pout;


}