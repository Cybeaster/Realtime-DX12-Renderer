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
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC : TEXCOORD;
	float2 WaveNormalTex0 : TEXCOORDNORMAL0;
	float2 WaveNormalTex1 : TEXCOORDNORMAL1;

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

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(Vin.TexC, 0.0f, 1.0f), texTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

    float2 tiledTexC = Vin.TexC * 64;
    float dampingStrength = 1.f;
    // Wave parameters for more complex and realistic waves
    float amplitude1 = 0.5f;
    float frequency1 = 4.0f;
    float speed1 = 0.3f;

    float amplitude2 = 0.3f;
    float frequency2 = 6.0f;
    float speed2 = 0.3f;

    float amplitude3 = 0.1f;
    float frequency3 = 8.0f;
    float speed3 = 0.4f;

    // Compute local wave coordinates for multiple wave patterns
    float2 coord1 = Vin.TexC * frequency1 + float2(gTotalTime * speed1, 0.0f);
    float2 coord2 = Vin.TexC * frequency2 + float2(gTotalTime * speed2, 0.0f);
    float2 coord3 = Vin.TexC * frequency3 + float2(gTotalTime * speed3, 0.0f);

    // Apply wave function for each wave pattern
    float wave1 = amplitude1 * sin(coord1.x);
    float wave2 = amplitude2 * sin(coord2.x);
    float wave3 = amplitude3 * sin(coord3.x);

    // Combine wave patterns for a more complex wave effect
    float combinedWave = wave1 + wave2 + wave3;
    // Adjust tiled texture coordinates for wave effect
    vout.WaveNormalTex0 = tiledTexC + float2(0, combinedWave);
    vout.WaveNormalTex1 = tiledTexC - float2(0, combinedWave);

    // Base wave calculations as before
    float wave = amplitude1 * sin(coord1.x) + amplitude2 * sin(coord2.x) + amplitude3 * sin(coord3.x);

    // Sample the height map
    float baseHeight = ComputeHeightMaps(matData, Vin.NormalL, Vin.TangentU, Vin.TexC, 1.0).r;

    // Calculate damping factor based on wave height
    float dampingFactor = 1.0 / (1.0 + abs(wave) * dampingStrength);

    // Apply damping to the wave height
    wave *= dampingFactor;

    // Combine the base height with the damped wave for the final vertex height
    Vin.PosL.y = (baseHeight * 5) + wave;

	// Transform to world space.

	float4 posW = mul(float4(Vin.PosL, 1.0f), world);
	vout.PosW = posW.xyz;


	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.

	vout.NormalW = mul(Vin.NormalL, (float3x3)world);

	if (!IsTangentValid(Vin.TangentU))
	{
		vout.TangentW = float3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		vout.TangentW = mul(Vin.TangentU, (float3x3)world);
	}

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);




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

	float3 bumpedNormalW = pin.NormalW;
	float4 normalMapSample = float4(0.f, 0.f, 1.0f, 1.0f);
	pin.NormalW = normalize(pin.NormalW);
	bumpedNormalW = ComputeNormalMaps(pin.NormalW, pin.TangentW, matData, pin.WaveNormalTex0, normalMapSample.a);
	diffuseAlbedo = ComputeDiffuseMaps(matData, diffuseAlbedo, pin.WaveNormalTex0);

	// Vector from point being lit to eye.
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize

	// Light terms.
	float4 ambient = gAmbientLight * diffuseAlbedo;

	const float shininess = (1.0f - roughness) * normalMapSample.a;
	Material mat = { diffuseAlbedo, fresnelR0, shininess };
	float3 shadowFactor = 1.0f;

	float3 light = {0.0f, 0.0f, 0.0f};

	 for (uint i = 0; i < gNumDirLights; i++)
	{
        light += shadowFactor* ComputeDirectionalLight(gDirectionalLights[i], mat, bumpedNormalW, toEyeW);
    }

     for (uint j = 0; j < gNumPointLights; j++)
    {
        light += ComputePointLight(gPointLights[j], mat, pin.PosW, bumpedNormalW, toEyeW);
    }

    for (uint k = 0; k < gNumSpotLights; k++)
    {
        light += ComputeSpotLight(gSpotLights[k], mat, pin.PosW, bumpedNormalW, toEyeW);
    }

	float4 litColor = ambient + float4(light, 1.0f);

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