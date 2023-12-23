
cbuffer cbPerObject : register(b0)
{
	float4x4 WorldViewProj;
};

cbuffer cbTimeObject : register(b1)
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
};

struct VertexIn
{
	float3 PosL : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{

	VertexOut vout;

	// Transform to homogeneous clip space.
	float4 posW = mul(float4(vin.PosL, 1.0f), World);
	vout.PosH = mul(posW, ViewProj);

	// Just pass vertex color into the pixel shader.
	vout.Color = vin.Color;
	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
	return pin.Color;
}
