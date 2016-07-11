cbuffer cbTrans
{
	float4x4 wvp;
    float4x4 world;
};

struct VertexIn
{
	float3 posL:POSITION;
	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
	float3 tangent:TANGENT;
};

struct VertexOut
{
	float4 posH:SV_POSITION;
	float3 posW:POSITION;
	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
	float3 tangent:TANGENT;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.posH = mul(float4(vin.posL, 1.f), wvp);
    vout.posW = mul(float4(vin.posL, 1.f), world).xyz;
	vout.normal = mul(float4(vin.normal,0.f),world).xyz;
	vout.tex = vin.tex;
	vout.tangent = vin.tangent;
	return vout;
}