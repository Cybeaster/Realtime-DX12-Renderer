

#include "LightingUtils.hlsl"

Texture2D DiffuseMap : register(t0);

SamplerState SampPointWrap : register(s0);
SamplerState SampPointClamp : register(s1);
SamplerState SampLinearWrap : register(s2);
SamplerState SampLinearClamp : register(s3);
SamplerState SampAnisotropicWrap : register(s4);
SamplerState SampAnisotropicClamp : register(s5);

cbuffer CbPerObject : register(b0)
{
	float4x4 World;
	float4x4 TexTransform;
};

// Constant data that varies per material.
cbuffer CbPass : register(b1)
{
	float4x4 View;
	float4x4 InvView;
	float4x4 Proj;
	float4x4 InvProj;
	float4x4 ViewProj;
	float4x4 InvViewProj;
	float3 EyePosW;
	float CbPerObjectPad1;
	float2 RenderTargetSize;
	float2 InvRenderTargetSize;
	float NearZ;
	float FarZ;
	float TotalTime;
	float DeltaTime;
	float4 AmbientLight;

	float4 FogColor;
	float FogStart;
	float FogRange;
	float2 CbPerObjectPad2;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};

cbuffer CbMaterial : register(b2)
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MatTransform;
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to world space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), World);

	return vout;
}

struct PatchTess
{
	float EdgeTess[4] : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

struct HullOut
{
	float3 PosL : POSITION;
};

PatchTess ConstantHS(InputPatch<VertexOut, 16> Patch, uint PatchID
                     : SV_PrimitiveID)
{
	PatchTess pt;

	// Compute the tessellation factors.
	pt.EdgeTess[0] = 10;
	pt.EdgeTess[1] = 10;
	pt.EdgeTess[2] = 10;
	pt.EdgeTess[3] = 10;

	pt.InsideTess[0] = 25;
	pt.InsideTess[1] = 25;

	return pt;
}

[domain("quad")]
    [partitioning("integer")]
    [outputtopology("triangle_cw")]
    [outputcontrolpoints(16)]
    [patchconstantfunc("ConstantHS")]
    [maxtessfactor(64.0f)]

    HullOut
    HS(InputPatch<VertexOut, 16> Patch,
       uint PointId
       : SV_OutputControlPointID,
         uint PatchId
       : SV_PrimitiveID)
{
	HullOut vout;

	// Just pass through.
	vout.PosL = Patch[PointId].PosH.xyz;

	return vout;
}

struct DomainOut
{
	float4 PosH : SV_POSITION;
};

float4 BernsteinBasis(float t)
{
	float invT = 1.0f - t;
	return float4(invT * invT * invT,
	              3.0f * invT * invT * t,
	              3.0f * invT * t * t,
	              t * t * t);
}

float3 CubicBezierSum(const OutputPatch<HullOut, 16> bezpatch, float4 basisU, float4 basisV)
{
	float3 sum = float3(0.0f, 0.0f, 0.0f);
	sum = basisV.x * (basisU.x * bezpatch[0].PosL + basisU.y * bezpatch[1].PosL + basisU.z * bezpatch[2].PosL + basisU.w * bezpatch[3].PosL);
	sum += basisV.y * (basisU.x * bezpatch[4].PosL + basisU.y * bezpatch[5].PosL + basisU.z * bezpatch[6].PosL + basisU.w * bezpatch[7].PosL);
	sum += basisV.z * (basisU.x * bezpatch[8].PosL + basisU.y * bezpatch[9].PosL + basisU.z * bezpatch[10].PosL + basisU.w * bezpatch[11].PosL);
	sum += basisV.w * (basisU.x * bezpatch[12].PosL + basisU.y * bezpatch[13].PosL + basisU.z * bezpatch[14].PosL + basisU.w * bezpatch[15].PosL);

	return sum;
}

float4 DBernsteinBasis(float t)
{
	float invT = 1.0f - t;
	return float4(-3 * invT * invT,
	              3 * invT * invT - 6 * t * invT,
	              6 * t * invT - 3 * t * t,
	              3 * t * t);
}

[domain("quad")] DomainOut
DS(PatchTess patchTess, float2 uv
   : SV_DomainLocation,
     const OutputPatch<HullOut, 16> quad) {
	DomainOut dout;

	// Bilinear interpolation.
	float4 basisU = BernsteinBasis(uv.x);
	float4 basisV = BernsteinBasis(uv.y);

	float3 p = CubicBezierSum(quad, basisU, basisV);

	float4 posW = mul(float4(p, 1.0f), World);
	dout.PosH = mul(posW, ViewProj);

	return dout;
}

float4 PS(DomainOut pin)
    : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
