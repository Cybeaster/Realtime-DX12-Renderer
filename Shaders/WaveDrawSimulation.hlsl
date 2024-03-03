

#include "Common.hlsl"

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD; // Existing texture coordinates
	float2 WaveTexC0 : WAVE_TEXCOORD0; // Additional texture coordinates for the first wave normal map
	float2 WaveTexC1 : WAVE_TEXCOORD1; // Additional texture coordinates for the second wave normal map
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC : TEXCOORD; // Existing texture coordinates
	float2 WaveTexC0 : WAVE_TEXCOORD0; // Pass-through for the first wave normal map
	float2 WaveTexC1 : WAVE_TEXCOORD1; // Pass-through for the second wave normal map
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

	// Transform to world space.
	float4 posW = mul(float4(Vin.PosL, 1.0f), world);

	vout.PosW = posW.xyz;
	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.NormalW = mul(Vin.NormalL, (float3x3)world);
	vout.TangentW = mul(Vin.TangentU, (float3x3)world);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(Vin.TexC, 0.0f, 1.0f), texTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

	vout.WaveTexC0 = Vin.WaveTexC0;
	vout.WaveTexC1 = Vin.WaveTexC1;
	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
	MaterialData matData = gMaterialData[pin.MaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
	float3 fresnelR0 = matData.FresnelR0;
	float roughness = matData.Roughness;
#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif

	float4 normalMapSample = float4(0.f, 0.f, 1.0f, 1.0f);
	pin.NormalW = normalize(pin.NormalW);

	float3 bumpedNormalW0 = ComputeNormalMap(0, pin.NormalW, pin.TangentW, matData, pin.WaveTexC0, normalMapSample.a);
	float3 bumpedNormalW1 = ComputeNormalMap(1, pin.NormalW, pin.TangentW, matData, pin.WaveTexC1, normalMapSample.a);
	float3 bumpedNormalW = normalize(bumpedNormalW0 + bumpedNormalW1);

	pin.PosW = ComputeHeightMaps(matData, pin.NormalW, pin.TangentW, pin.TexC, pin.PosW);
	diffuseAlbedo = ComputeDiffuseMaps(matData, diffuseAlbedo, pin.TexC);

	// Vector from point being lit to eye.
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize

	// Light terms.
	float4 ambient = gAmbientLight * diffuseAlbedo;

	const float shininess = (1.0f - roughness) * normalMapSample.a;
	Material mat = { diffuseAlbedo, fresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	float4 directLight = ComputeLighting(gLights, mat, pin.PosW, bumpedNormalW, toEyeW, shadowFactor);

	float4 litColor = ambient + directLight;

	float3 reflection = reflect(-toEyeW, bumpedNormalW);
	float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, reflection);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, toEyeW);
	litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount); //TODO fix fog
#endif
	// Common convention to take alpha from diffuse albedo.
	litColor.a = diffuseAlbedo.a;
	return litColor;
}
