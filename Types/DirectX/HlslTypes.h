
#ifndef HLSL
#pragma once
#define float3 DirectX::XMFLOAT3
#define float4 DirectX::XMFLOAT4
#define float2 DirectX::XMFLOAT2
#define float4x4 DirectX::XMFLOAT4X4
#define uint unsigned int

#include "DirectX/DXHelper.h"
namespace HLSL
{
#endif

#ifndef MAX_SHADOW_MAPS
#define MAX_SHADOW_MAPS 3
#endif

#define TEXTURE_MAPS_NUM 312
#define SHADOW_MAPS_NUM 1

#define EPSILON 0.0001f
#define F0_COEFF 0.16f
#define MAX_CSM_PER_FRAME 3

#define STRINGIFY(x) #x
#define STRINGIFY_MACRO(x) STRINGIFY(x)

#define MAKE_SHADER_LITERAL(Name)

#define CB_PASS cbPass
#define FRUSTRUM_CORNERS_BUFFER gFrustrumCorners

struct TextureData
{
	uint bIsEnabled;
	uint TextureIndex;
};

struct MaterialData
{
	float3 AmbientAlbedo;
	float3 DiffuseAlbedo;
	float3 SpecularAlbedo;
	float3 Transmittance;
	float3 Emission;
	float Shininess;
	float IndexOfRefraction;
	float Dissolve;
	int Illumination;
	float Roughness;
	float Metalness;
	float Sheen;

	float4x4 MatTransform;

	TextureData DiffuseMap;
	TextureData NormalMap;
	TextureData HeightMap;
	TextureData AlphaMap;
	TextureData SpecularMap;
	TextureData AmbientMap;
};

struct SpotLight
{
	float3 Position;
	float pad1; // Padding to align with HLSL's float4
	float3 Direction;
	float pad2; // Additional padding if necessary
	float3 Intensity;
	float FalloffStart;
	float FalloffEnd;
	float SpotPower;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float ConeAngle;
	float4x4 Transform;
};

struct PointLight
{
	float3 Position;
	float pad1; // Padding to align with HLSL's float4
	float3 Intensity;
	float FalloffStart;
	float FalloffEnd;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float2 pad2; // Padding to align with HLSL's float4
	float4x4 Transform;
};

//supposed to use cascade shadow mapping

struct ShadowMapData
{
	float4x4 Transform;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float MaxDistance;
	float2 pad; // Padding to align with HLSL's float4
};

struct DirectionalLight
{
	float3 Direction;
	float pad1; // Padding to align with HLSL's float4
	float3 Intensity;
	float pad2;
	ShadowMapData ShadowMapDataNear;
	ShadowMapData ShadowMapDataMid;
	ShadowMapData ShadowMapDataFar;
};

struct FrustrumCorners
{
	float3 Corners[8];
};

#ifndef HLSL
inline float3 FrustumCornens[8] = {
	float3(-1.0f, 1.0f, -1.0f),
	float3(1.0f, 1.0f, -1.0f),
	float3(1.0f, -1.0f, -1.0f),
	float3(-1.0f, -1.0f, -1.0f),
	float3(-1.0f, 1.0f, 1.0f),
	float3(1.0f, 1.0f, 1.0f),
	float3(1.0f, -1.0f, 1.0f),
	float3(-1.0f, -1.0f, 1.0f)
};
#endif
#ifndef HLSL
}
#endif
