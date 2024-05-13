#include "Common.hlsl"

struct VertexInput {
	float3 PosL : POSITION;
};

struct PixelInput {
	float4 PosH : SV_POSITION;
	nointerpolation float3 Color : COLOR;
};

PixelInput VS(VertexInput Vin,uint InstanceID : SV_InstanceID) {
	PixelInput vout;

	InstanceData inst = gInstanceData[InstanceID];
	float4x4 world = inst.World;

	// Transform to world space.
	float4 posW = mul(float4(Vin.PosL, 1.0f), world);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);
	vout.Color  = inst.OverrideColor;
	return vout;
}

float4 PS(PixelInput input) : SV_Target {
	return float4(input.Color,1);
}
