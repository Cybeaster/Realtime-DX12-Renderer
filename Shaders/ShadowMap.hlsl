
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 TexC    : TEXCOORD;
	nointerpolation uint MaterialIndex : MATERIALINDEX;
};

VertexOut VS(VertexIn vin,uint InstanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;

    InstanceData inst = gInstanceData[InstanceID];
	float4x4 world = inst.World;
	uint matIndex = inst.MaterialIndex;
	vout.MaterialIndex = matIndex;
	MaterialData matData = gMaterialData[matIndex];

    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), world);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), inst.TexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

    return vout;
}


// This is only used for alpha cut out geometry, so that shadows
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void PS(VertexOut pin)
{
	// Fetch the material data.
	MaterialData matData = gMaterialData[pin.MaterialIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;

	// Dynamically look up the texture in the array.
	diffuseAlbedo *= gTextureMaps[matData.DiffuseMapIndex[0]].Sample(gsamAnisotropicWrap, pin.TexC);

#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1.  We do this test as soon
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(diffuseAlbedo.a - 0.1f);
#endif
}