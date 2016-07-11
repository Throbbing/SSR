cbuffer cbToFarPlane
{
	float4	 farPlaneCorner[4];
};

struct VertexIn
{
	float3 posH:POSITION;
	float3 tex:TEXCOORD;
};

struct VertexOut
{
	float4 posH:SV_POSITION;
	float3 ray:RAY;
	float2 tex:TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.posH = float4(vin.posH, 1.f);
	//��Camera Space �У��������ߵ�ԭ�㶼Ϊ(0,0,0),���������
	vout.ray = normalize(farPlaneCorner[(uint)vin.tex.z].xyz);
	vout.tex = vin.tex.xy;
	return vout;
}