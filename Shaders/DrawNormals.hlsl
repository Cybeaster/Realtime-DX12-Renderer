#include "Common.hlsl"


struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
    float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC     : TEXCOORD;
	nointerpolation uint MaterialIndex : MATERIALINDEX;
};


VertexOut VS(VertexIn Vin, uint InstanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut)0.0f;
    InstanceData inst = gInstanceData[InstanceID];
    float4x4 world = inst.World;
    float4x4 texTransform = inst.TexTransform;
    uint matIndex = inst.MaterialIndex;

    MaterialData matData = gMaterialData[matIndex];

     // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(Vin.NormalL, (float3x3)world);
    vout.TangentW = mul(Vin.TangentU, (float3x3)world);
    float4 posW = mul(float4(Vin.PosL, 1.0f), world);
    vout.PosH = mul(posW, gViewProj);

    float4 texC = mul(float4(Vin.TexC, 0.0f, 1.0f), texTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
    vout.MaterialIndex = matIndex;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = gMaterialData[pin.MaterialIndex];
    float4 diffuseAlbedo = float4(matData.DiffuseAlbedo,1);
    diffuseAlbedo *= ComputeDiffuseMap(matData, pin.TexC);

#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif
    // Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);
	// Write normal in view space coordinates
    float3 normalV = mul(pin.NormalW, (float3x3)gView);
    return float4(normalV, 0.0f);
}