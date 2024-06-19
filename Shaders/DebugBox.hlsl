#include "Common.hlsl"

struct VertexInput
{
    float3 PosL : POSITION;
};

struct VertexOutput
{
	float4 PosH : SV_POSITION;
    float3 Color : COLOR;
};

VertexOutput VS(VertexInput Input, uint InstanceID : SV_InstanceID)
{
	VertexOutput output = (VertexOutput)0;
	InstanceData inst = gInstanceData[InstanceID];
	float4x4 world = inst.World;

	float4 posW = mul(float4(Input.PosL, 1.0f), world);
	output.PosH = mul(posW, cbCamera.ViewProj);
	output.Color = inst.OverrideColor;
	return output;
}

float4 PS(VertexOutput input) : SV_Target
{
	return float4(input.Color,1);
}