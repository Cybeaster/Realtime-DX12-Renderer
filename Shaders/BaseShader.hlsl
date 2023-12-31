// Defaults for number of lights.

#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtils.hlsl"

#define MaxLights 16


cbuffer cbPerObject : register(b0)
{
	float4x4 World;
};

cbuffer cbMaterial :register(b1)
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MatTransform;
};

// Constant data that varies per material.
cbuffer cbPass : register(b2)
{
	float4x4 View;
	float4x4 InvView;
	float4x4 Proj;
	float4x4 InvProj;
	float4x4 ViewProj;
	float4x4 InvViewProj;

	float3 EyePosW;
	float cbPerObjectPad1;
	float2 RenderTargetSize;
	float2 InvRenderTargetSize;
	float NearZ;
	float FarZ;
	float TotalTime;
	float DeltaTime;
	float4 AmbientLight;

	Light Lights[MaxLights];
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), World);
	vout.PosW = posW.xyz;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.NormalW = mul(vin.NormalL, (float3x3)World);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, ViewProj);

	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
    // Interpolating normal can unnormalize it, so renormalize it.
	pin.NormalW = normalize(pin.NormalW);

	// Vector from point being lit to eye.
	float3 toEyeW = normalize(EyePosW - pin.PosW);

	// Indirect lighting.
	float4 ambient = AmbientLight * DiffuseAlbedo;

	const float shininess = 1.0f - Roughness;
	Material mat = { DiffuseAlbedo, FresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	float4 directLight = ComputeLighting(Lights,mat,pin.PosW,pin.NormalW,toEyeW,shadowFactor);
	float4 litColor = ambient + directLight;
	litColor.a = DiffuseAlbedo.a;
	return litColor;
}

