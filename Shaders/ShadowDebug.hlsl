#include "Common.hlsl"

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	 float4 PosH : SV_POSITION;
	 float2 TexC : TEXCOORD;
};

Texture2D gNormalMap : register(t3,  space3);
Texture2D gDepthMap  : register(t4,  space3);

VertexOut VS(VertexIn vin, uint instanceID
              : SV_InstanceID)
{
	 VertexOut vout;
	 // Directly output the position in screen space (assuming PosL is already in screen coordinates)

	 vout.PosH = float4(vin.PosL, 1.f);
	 vout.TexC = vin.TexC;
	 return vout;
}

float4 PS(VertexOut pin)
     : SV_Target
{
	 return float4(gShadowMaps[0].Sample(gsamLinearWrap, pin.TexC).rrr, 1.0f);
}
