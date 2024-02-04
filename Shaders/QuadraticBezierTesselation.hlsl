

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

PatchTess ConstantHS(InputPatch<VertexOut, 9> Patch, uint PatchID
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
    [outputcontrolpoints(9)]
    [patchconstantfunc("ConstantHS")]
    [maxtessfactor(64.0f)]

    HullOut
    HS(InputPatch<VertexOut, 9> Patch,
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

float3 QuadraticBernsteinBasis(float t)
{
	float u = t; // Parameter for the curve
	float invU = 1.0f - u; // Inverse of the parameter
	return float3(invU * invU, 2 * invU * u, u * u);
}

float3 QuadraticBezierSum(const OutputPatch<HullOut, 9> patch, float3 basisU, float3 basisV)
{
	float3 sum = float3(0.0f, 0.0f, 0.0f);

	// Row 1
	sum += basisV.x * (basisU.x * patch[0].PosL + basisU.y * patch[1].PosL + basisU.z * patch[2].PosL);
	// Row 2
	sum += basisV.y * (basisU.x * patch[3].PosL + basisU.y * patch[4].PosL + basisU.z * patch[5].PosL);
	// Row 3
	sum += basisV.z * (basisU.x * patch[6].PosL + basisU.y * patch[7].PosL + basisU.z * patch[8].PosL);

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

[domain("quad")] DomainOut DS(PatchTess patchTess, float2 uv
                              : SV_DomainLocation, const OutputPatch<HullOut, 9> quad) {
	DomainOut dout;

	// Compute the quadratic Bernstein basis functions for u and v.
	float3 basisU = QuadraticBernsteinBasis(uv.x);
	float3 basisV = QuadraticBernsteinBasis(uv.y);

	// Compute the position on the quadratic Bézier surface.
	float3 p = QuadraticBezierSum(quad, basisU, basisV);

	// Transform to world space and then to homogeneous clip space.
	float4 posW = mul(float4(p, 1.0f), World);
	dout.PosH = mul(posW, ViewProj);

	return dout;
}

float4 PS(DomainOut pin)
    : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
