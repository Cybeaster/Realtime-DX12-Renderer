// Combines two images.
//***************************************************************************************

Texture2D BaseMap : register(t0);
Texture2D EdgeMap : register(t1);

SamplerState SamPointWrap : register(s0);
SamplerState SamPointClamp : register(s1);
SamplerState SamLinearWrap : register(s2);
SamplerState SamLinearClamp : register(s3);
SamplerState SamAnisotropicWrap : register(s4);
SamplerState SamAnisotropicClamp : register(s5);

static const float TexCoords[6] = {
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD;
};

VertexOut VS(uint vid
             : SV_VertexID)
{
	VertexOut vout;
	vout.TexC = TexCoords[vid];

	// Map [0,1]^2 to NDC space.
	vout.PosH = float4(vout.TexC * 2.0f - 1.0f, 0.0f, 1.0f);
	return vout;
}

float4 PS(VertexOut pin
          : SV_POSITION)
    : SV_Target
{
	float4 baseColor = BaseMap.Sample(SamLinearClamp, pin.TexC);
	float4 edgeColor = EdgeMap.Sample(SamLinearClamp, pin.TexC);

	return baseColor * edgeColor;
}