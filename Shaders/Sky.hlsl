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

VertexOut VS(VertexIn vin, uint InstanceID
             : SV_InstanceID)
{
	InstanceData inst = gInstanceData[InstanceID];
	VertexOut vout;
	vout.PositionL = vin.Position;
	float4 posW = mul(float4(vin.Position, 1.0f), inst.World);
	posW.xyz += cbCamera.EyePosW;
	vout.PositionH = mul(posW, cbCamera.ViewProj).xyww;
	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
	return gCubeMap.Sample(gsamLinearWrap, pin.PositionL) * (cbCamera.AmbientLight * 5.f);
}