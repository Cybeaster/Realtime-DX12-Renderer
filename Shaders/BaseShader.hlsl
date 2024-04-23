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
	float3 PosW : POSITION;
    float4 SsaoPosH   : POSITION1;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC : TEXCOORD;
	float4 ShadowPositionsH[MAX_SHADOW_MAPS] : TEXCOORD3;
	nointerpolation uint LightIndices[MAX_SHADOW_MAPS] : TEXCOORD1;
	nointerpolation uint MaterialIndex : MATERIALINDEX;
};

bool ValidateTangent(VertexIn input)
{
    // Check if the normal and tangent are normalized
    if (!IsNormalized(input.NormalL) || !IsNormalized(input.TangentU)) {
        return false;
    }

    // Check if the normal and tangent are orthogonal
    if (!AreOrthogonal(input.NormalL, input.TangentU)) {
        return false;
    }

    // Optionally check the bitangent
    float3 bitangent = cross(input.NormalL, input.TangentU);
    return IsNormalized(bitangent) && AreOrthogonal(bitangent, input.NormalL) && AreOrthogonal(bitangent, input.TangentU);
}

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
    FindShadowPosition(vout.ShadowPositionsH, posW, vout.LightIndices);
    vout.SsaoPosH = mul(posW, gViewProjTex);

	return vout;
}

float4 PS(VertexOut pin)
    : SV_Target
{
	MaterialData matData = gMaterialData[pin.MaterialIndex];
	float alpha = SampleAlphaMap(matData, pin.TexC);
#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(alpha - 0.1f);
#endif

	float3 diffuseAlbedo = matData.DiffuseAlbedo * SampleDiffuseMap(matData, pin.TexC).rgb;
	float3 specularAlbedo = matData.SpecularAlbedo * SampleSpecularMap(matData, pin.TexC).rgb;

	pin.NormalW = normalize(pin.NormalW);
	pin.TangentW = normalize(pin.TangentW);

	float reflectance = CalcFresnelR0(matData.IndexOfRefraction); //TODO fix if IndexOfRefraction == 0
    float3 f0 = lerp(F0_COEFF * SQUARE(reflectance), diffuseAlbedo,  matData.Metalness);

	// Vector from point being lit to eye.
	float3 toEyeW = normalize(gEyePosW - pin.PosW);
	float distToEye = length(toEyeW);

	float ambientAccess = 1;
	if (gSSAOEnabled)
	{
		pin.SsaoPosH /= pin.SsaoPosH.w;
		ambientAccess = gSsaoMap.Sample(gsamLinearWrap, pin.SsaoPosH.xy, 0.0f).r;
	}

    float shadowFactor[MAX_SHADOW_MAPS];
    GetShadowFactor(shadowFactor,  pin.LightIndices,pin.ShadowPositionsH);

	//diffuse incorporating metalness
	Material mat = { diffuseAlbedo * (1.0f - matData.Metalness), f0.rgb, matData.Shininess, 1 -  (matData.Shininess / 100) };
	BumpedData data = SampleNormalMap(pin.NormalW, pin.TangentW, matData, pin.TexC);
	float3 bumpedNormalW = data.BumpedNormalW;

    uint idx = 0;
	float3 directLighting = {0.0f, 0.0f, 0.0f};
	for (uint i = 0; i < gNumDirLights; i++)
	{
		DirectionalLight curLight = gDirectionalLights[i];
        directLighting += shadowFactor[idx] * ComputeDirectionalLight_BRDF(curLight, mat, bumpedNormalW, toEyeW);
        idx++;
    }
	directLighting *= 10;
    return float4(directLighting,1);

	// Light terms.
	float4 ambient =  ambientAccess * gAmbientLight * float4(diffuseAlbedo,1.0);
	float4 litColor = ambient + float4(directLighting, 1.0f);

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount); //TODO fix fog
#endif

	// Common convention to take alpha from diffuse albedo.
	litColor.a = alpha;
	litColor.rgb = AcesFitted(litColor.rgb);
	litColor.rgb = GammaCorrect(litColor.rgb);
	return litColor;
}
