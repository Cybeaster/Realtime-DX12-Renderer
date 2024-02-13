
// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS

#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS

#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS

#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.hlsl"

struct InstanceData
{
	float4x4 World;
	float4x4 TexTransform;
	uint MaterialIndex;
	float2 DisplacementMapTexelSize;
	float GridSpatialStep;
};

struct MaterialData
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MatTransform;
	uint DiffuseMapIndex;
	uint MatPad0;
	uint MatPad1;
	uint MatPad2;
};

Texture2D gDiffuseMap[7] : register(t0);
Texture2D gDisplacementMap : register(t7);

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);
// Put in space1, so the texture array does not overlap with these resources.
// The texture array will occupy registers t0, t1, ..., t3 in space0.
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

// Constant data that varies per material.
cbuffer cbPass : register(b0)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float3 gEyePosW;
	float cbPerPassPad1;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;

	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 cbPerPassPad2;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;

	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;

	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)

	// are spot lights for a maximum of MaxLights per object.

	Light gLights[MaxLights];
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float2 TexC : TEXCOORD;

	nointerpolation uint MaterialIndex : MATERIALINDEX;
};

VertexOut VS(VertexIn Vin, uint InstanceID
             : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;

	InstanceData inst = gInstanceData[InstanceID];
	float4x4 world = inst.World;
	float4x4 texTransform = inst.TexTransform;
	uint matIndex = inst.MaterialIndex;
	vout.MaterialIndex = matIndex;
	MaterialData matData = gMaterialData[matIndex];

#ifdef DISPLACEMENT_MAP
	// Sample the displacement map using non-transformed [0,1]^2 tex-coords.
	Vin.PosL.y += gDisplacementMap.SampleLevel(gsamLinearWrap, Vin.TexC, 1.0f).r;

	// Estimate normal using finite difference.
	float du = inst.DisplacementMapTexelSize.x;
	float dv = inst.DisplacementMapTexelSize.y;
	float l = gDisplacementMap.SampleLevel(gsamPointClamp, Vin.TexC - float2(du, 0.0f), 0.0f).r;
	float r = gDisplacementMap.SampleLevel(gsamPointClamp, Vin.TexC + float2(du, 0.0f), 0.0f).r;
	float t = gDisplacementMap.SampleLevel(gsamPointClamp, Vin.TexC - float2(0.0f, dv), 0.0f).r;
	float b = gDisplacementMap.SampleLevel(gsamPointClamp, Vin.TexC + float2(0.0f, dv), 0.0f).r;
	Vin.NormalL = normalize(float3(-r + l, 2.0f * inst.GridSpatialStep, b - t));
#endif

	// Transform to world space.

	float4 posW = mul(float4(Vin.PosL, 1.0f), world);
	vout.PosW = posW.xyz;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.

	vout.NormalW = mul(Vin.NormalL, (float3x3)world);

	// Transform to homogeneous clip space.

	vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(Vin.TexC, 0.0f, 1.0f), texTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
	MaterialData matData = gMaterialData[pin.MaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
	float3 fresnelR0 = matData.FresnelR0;
	float roughness = matData.Roughness;
	uint diffuseMapIndex = matData.DiffuseMapIndex;

	// Dynamically look up the texture in the array.
	diffuseAlbedo *= gDiffuseMap[diffuseMapIndex].Sample(gsamLinearWrap, pin.TexC);
#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif

	// Interpolating normal can unnormalize it, so renormalize it.

	pin.NormalW = normalize(pin.NormalW);

	// Vector from point being lit to eye.
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize

	// Light terms.

	float4 ambient = gAmbientLight * diffuseAlbedo;

	const float shininess = 1.0f - roughness;
	Material mat = { diffuseAlbedo, fresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

	float4 litColor = ambient + directLight;

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif

	// Common convention to take alpha from diffuse albedo.

	litColor.a = diffuseAlbedo.a;
	return litColor;
}
