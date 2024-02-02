

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
	float2 DisplacementMapTexelSize;
	float GridSpatialStep;
	float cbPerObjectPad1;
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
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
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
	float3 PosL : POSITION;
};

VertexOut VS(VertexIn Vin)
{
	VertexOut Vout;
	Vout.PosL = Vin.PosL;
	return Vout;
}

struct PatchTess
{
	float Edges[4] : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> Patch, uint PatchID
                     : SV_PrimitiveID)
{
	PatchTess pt;
	float3 centerL = 0.25f * (Patch[0].PosL + Patch[1].PosL + Patch[2].PosL + Patch[3].PosL);
	float3 centerW = mul(float4(centerL, 1.0f), World).xyz;

	float d = distance(centerW, EyePosW);

	// Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.

	const float d0 = 20.f;
	const float d1 = 100.f;
	float tess = 64 * saturate((d1 - d) / (d1 - d0));

	// Uniformly tessellate the patch.
	pt.Edges[0] = tess;
	pt.Edges[1] = tess;
	pt.Edges[2] = tess;
	pt.Edges[3] = tess;

	// No interior tessellation.
	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;
	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
};

[domain("quad")]
    [partitioning("fractional_odd")]
    [outputtopology("triangle_cw")]
    [outputcontrolpoints(4)]
    [patchconstantfunc("ConstantHS")]
    [maxtessfactor(64.0f)]

    HullOut
    HS(InputPatch<VertexOut, 4> Patch, uint PointID
       : SV_OutputControlPointID,
         uint PatchID
       : SV_PrimitiveID)
{
	HullOut vout;
	vout.PosL = Patch[PointID].PosL;
	return vout;
}

// The domain shader is called for every vertex created by the tessellator.
// It is like the vertex shader after tessellation.
struct DomainOut
{
	float4 PosH : SV_POSITION;
};

[domain("quad")] DomainOut
DS(PatchTess patchTess, float2 uv
   : SV_DomainLocation,
     const OutputPatch<HullOut, 4> quad) {
	DomainOut dout;

	// Bilinear interpolation.
	float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
	float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
	float3 p = lerp(v1, v2, uv.y);

	// Displacement mapping
	p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z));

	float4 posW = mul(float4(p, 1.0f), World);
	dout.PosH = mul(posW, ViewProj);

	return dout;
}

float4 PS(DomainOut pin)
    : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}