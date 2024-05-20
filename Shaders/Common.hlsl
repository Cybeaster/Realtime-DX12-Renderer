// Include structures and functions for lighting.
#include "LightingUtils.hlsl"
#include "Samplers.hlsl"

Texture2D gTextureMaps[TEXTURE_MAPS_NUM] : register(t0,space0);
Texture2D gShadowMaps[MAX_SHADOW_MAPS] : register(t1,space2);

TextureCube gCubeMap : register(t1,space3);
Texture2D gSsaoMap   : register(t2,space3);

StructuredBuffer<InstanceData> gInstanceData : register(t0, space4);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space4);

StructuredBuffer<SpotLight> gSpotLights : register(t2, space4);
StructuredBuffer<PointLight> gPointLights : register(t3, space4);
StructuredBuffer<DirectionalLight> gDirectionalLights : register(t4, space4);


cbuffer CB_PASS : register(b0)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gViewProjTex;
	float3 gEyePosW;

	float cbPerPassPad1; // Use this to pad gEyePosW to 16 bytes

	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;
	float4 gFogColor;
	float gFogStart;
	float gFogRange;

	uint gNumDirLights;

	float cbPerPassPad2; // Padding to align following uints
	uint gNumPointLights;
	uint gNumSpotLights;
	float cbPerPassPad3; // Padding to ensure the cbuffer ends on a 16-byte boundary
	float cbPerPassPad4; // Padding to ensure the cbuffer ends on a 16-byte boundary
	bool gSSAOEnabled;
};


bool IsTangentValid(float3 TangentW)
{
    float sum = abs(TangentW.x) + abs(TangentW.y) + abs(TangentW.z);
    return sum > EPSILON;
}

float3 NormalSampleToWorldSpace(float3 NormalMapSample, float3 UnitNormalW, float3 TangentW)
{
	float3 normalT = NormalMapSample * 2.0f - 1.0f;
	float3 N = UnitNormalW;
	/*
	*  This code makes sure T is orthonormal to N by subtracting off any
	    component of T along the direction N
	*/
	float3 T = normalize(TangentW - dot(TangentW, N) * N); //gramm shmidt
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	float3 bumpedNormal = mul(normalT, TBN);
	return normalize(bumpedNormal);
}

struct BumpedData
{
    float4 NormalMapSample;
	float3 BumpedNormalW;
};


float3 ComputeHeightMap(MaterialData matData, float3 NormalW, float3 TangentW, float2 TexC,float Offset )
{
	if(matData.HeightMap.bIsEnabled == 0)
	{
		return gTextureMaps[ matData.HeightMap.TextureIndex].SampleLevel(gsamAnisotropicWrap, TexC,Offset).r;
	}
	return float3(0.0f,0.0f,0.0f);
}

bool IsShadowCoordsWithinRange(float3 ShadowCoords)
{
	 return all(ShadowCoords.xy >= SHADOW_EPSILON) && all(ShadowCoords.xy <= 1 - SHADOW_EPSILON);
}

float2 GetShadowMapResolution(uint TexID)
{
	uint width, height, numMips;
    gShadowMaps[TexID].GetDimensions(0, width, height, numMips);
    return float2(width, height);

}
bool FindDirLightShadowPosition(float4 PosW, out float3 ShadowPosition, out uint LightIndex)
{
	LightIndex = 0;
	ShadowPosition = float3(0.0f,0.0f,0.0f);
    if(gNumDirLights > 0)
    {
        DirectionalLight light = gDirectionalLights[0];
        for (uint i = 0; i < MAX_CSM_PER_FRAME; ++i)
		{
			LightIndex = light.ShadowMapData[i].ShadowMapIndex;
			ShadowPosition = mul(PosW, light.ShadowMapData[i].Transform).xyz;
			if(IsShadowCoordsWithinRange(ShadowPosition))
			{
				return true;
			}
		}
	}
	return false;
}


bool FindSpotLightShadowPosition(float4 PosW, out float3 ShadowPosition, out uint LightIndex)
{
	LightIndex = 0;
	ShadowPosition = float3(0.0f,0.0f,0.0f);
    if(gNumSpotLights > 0)
    {
        for (uint i = 0; i < gNumSpotLights; ++i)
        {
            SpotLight light = gSpotLights[i];
            LightIndex = i;
            ShadowPosition = mul(PosW, light.Transform).xyz;
            if(IsShadowCoordsWithinRange(ShadowPosition))
            {
                return true;
            }
        }
    }
    return false;
}



float CalcShadowFactor(float3 ShadowPosH,uint ShadowMapIndex)
{

    //depth in NDC space
    float depth = ShadowPosH.z;

    Texture2D shadowMap = gShadowMaps[ShadowMapIndex];
    //Get shadow map dimensions
    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);

    //texel size
    float dx = 1.0f / (float)width;
    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
       percentLit += shadowMap.SampleCmpLevelZero(gsamShadow,
                  ShadowPosH.xy + offsets[i], depth + 0.005).r;
    }
    return percentLit / 9.0f;
}

void GetShadowFactor(out float ShadowFactors[SHADOW_MAPS_NUM],uint ShadowMapIndices[SHADOW_MAPS_NUM] ,float4 ShadowPositions[SHADOW_MAPS_NUM])
{
   // for (uint i = 0; i < SHADOW_MAPS_NUM; ++i)
    //{
    //    ShadowFactors[i] = CalcShadowFactor(ShadowPositions[i],ShadowMapIndices[i]);;
   // }
}

bool IsNormalized(float3 vector)
{
    float lengthSquared = dot(vector, vector);
    return abs(lengthSquared - 1.0) < EPSILON;
}

bool AreOrthogonal(float3 vec1, float3 vec2)
 {
    float dotProduct = dot(vec1, vec2);
    return abs(dotProduct) < EPSILON;
}

float SampleAlphaMap(MaterialData Data, float2 TexC)
{
	if(Data.AlphaMap.bIsEnabled == 1)
	{
		return gTextureMaps[Data.AlphaMap.TextureIndex].Sample(gsamAnisotropicWrap, TexC).r;
	}
	else
	{
		return Data.Dissolve;
	}
}

float4 SampleSpecularMap(MaterialData Data, float2 TexC)
{
	if(Data.SpecularMap.bIsEnabled == 1)
	{
		return gTextureMaps[Data.SpecularMap.TextureIndex].Sample(gsamAnisotropicWrap, TexC);
	}
	else
	{
		return float4(1.0,1.0,1.0,1.0);
	}
}

float4 SampleAmbientMap(MaterialData Data, float2 TexC)
{
	if(Data.AmbientMap.bIsEnabled == 1)
	{
		return gTextureMaps[Data.AmbientMap.TextureIndex].Sample(gsamAnisotropicWrap, TexC);
	}
	else
	{
		return float4(1.0,1.0,1.0,1.0);
	}
}

float4 SampleDiffuseMap(MaterialData MatData, float2 TexC)
{
	if(MatData.DiffuseMap.bIsEnabled == 1)
	{
		return gTextureMaps[MatData.DiffuseMap.TextureIndex].Sample(gsamAnisotropicWrap, TexC);
	}
	else
	{
		return float4(1.0,1.0,1.0,1.0);
	}
}

BumpedData SampleNormalMap(float3 NormalW, float3 TangentW, MaterialData MatData, float2 TexC)
{
	BumpedData data = (BumpedData)0.0;
	data.BumpedNormalW = NormalW;
	data.NormalMapSample = float4(0.f, 0.f, 0.0f, 1.0f);
  	if(MatData.NormalMap.bIsEnabled == 1)
  	{
		data.NormalMapSample = gTextureMaps[MatData.NormalMap.TextureIndex].Sample(gsamAnisotropicWrap, TexC);
		data.BumpedNormalW = NormalSampleToWorldSpace(data.NormalMapSample.rgb, NormalW, TangentW);
  	}
  	return data;
}

float3 GammaCorrect(float3 Color)
{
    return pow(Color, 1.0 / 2.2);
}

float CalcFresnelR0(float Ior)
{
    return pow((1.0 - Ior) / (1.0 + Ior), 2.0);
}

float3 ReinhardToneMapping(float3 Color)
{
	return Color / (Color + 1.0);
}

float3 ReinhardExtendedToneMapping(float3 Color, float MaxWhite)
{
	float3 num = Color * (1.0f + (Color / FLOAT3(MaxWhite * MaxWhite)));
	return num / (1.0f + Color);
}

float Luminance(float3 Color)
{
	return dot(Color, float3(0.2126, 0.7152, 0.0722));
}

float3 ChangeLuminance(float3 Color, float NewLuminance)
{
	float currentLuminance = Luminance(Color);
	return Color * (NewLuminance / currentLuminance);
}

float3 ReinhardExtendedLuminance(float3 Color, float MaxWhite)
{
	float luminance = Luminance(Color);
	float num = luminance * (1.0  + ( luminance / (MaxWhite * MaxWhite)));
	float lNew = num / (1.0 + luminance);
	return ChangeLuminance(Color, lNew);
}
