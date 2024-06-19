#include "Common.hlsl"

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
	float3 TangentU : TANGENT;
};


struct GSInput
{
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
};

struct GSOutput
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

GSInput VS(VertexIn Vin, uint InstanceID
			 : SV_InstanceID)
{
	GSInput gin = (GSInput)0.0f;

	InstanceData inst = gInstanceData[InstanceID];
	float4x4 world = inst.World;

	float4 posW = mul(float4(Vin.PosL, 1.0f), world);
	gin.PosW = posW.xyz;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	gin.NormalW = mul(Vin.NormalL, (float3x3)world);
	gin.TangentW = mul(Vin.TangentU, (float3x3)world);

	return gin;
}

[maxvertexcount(12)]
void GS(triangle GSInput In[3], inout LineStream<GSOutput> OutputStream)
{
	const float scale = 10.f;  // Adjust scale for visibility
	GSOutput Out;

	for (int i = 0; i < 3; ++i)
	{
		float4 posH = mul(float4(In[i].PosW, 1.0), cbCamera.ViewProj);
		// Position
		Out.PosH = posH;

		// Normal visualization
		Out.Color = float4(0, 1, 0, 1);  // Green for normals
		OutputStream.Append(Out);
		Out.PosH = mul(float4(In[i].PosW + In[i].NormalW * scale, 1.0), cbCamera.ViewProj);
		OutputStream.Append(Out);
		OutputStream.RestartStrip();

		// Tangent visualization
		Out.PosH = posH;
		Out.Color = float4(1, 0, 0, 1);  // Red for tangents
		OutputStream.Append(Out);
		Out.PosH = mul(float4(In[i].PosW + In[i].TangentW * scale, 1.0), cbCamera.ViewProj);
		OutputStream.Append(Out);
		OutputStream.RestartStrip();
	}
}


float4 PS(GSOutput pin) : SV_Target
{
	return pin.Color;
}