cbuffer cbTrans
{

	float4x4 wvp;
	float4x4 worldView;
};

struct VertexIn
{
	float3 posL:POSITION;
	float3 normalL:NORMAL;
	float2 tex:TEXCOORD;
	float3 tangent:TANGENT;
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

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.posH = mul(float4(vin.posL, 1.f), wvp);
	vout.posV = mul(float4(vin.posL, 1.f), worldView).xyz;
	vout.normalV = mul(float4(vin.normalL, 0.f), worldView).xyz;
	vout.normalW = vin.normalL;
	vout.tex = vin.tex;
	vout.tangentW = vin.tangent;
	return vout;
}

