#include "Common.hlsl"

struct VertexIn
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
	float4 PositionH : SV_POSITION;
	float3 PositionL : POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PositionH = vin.PositionL;
	float4 posW = mul(float4(vin.PositionL, 1.0f), gWorld);
	posW.xyz += gEyePosW;
	posW = mul(posW, gViewProj).xyww;
	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
	return gCubeMap.Sample(gsamLinearWrap, pin.PositionL);
}