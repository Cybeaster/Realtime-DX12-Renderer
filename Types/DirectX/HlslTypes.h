
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

#ifndef MAX_DIFFUSE_MAPS
#define MAX_DIFFUSE_MAPS 3
#endif

#ifndef MAX_NORMAL_MAPS
#define MAX_NORMAL_MAPS 3
#endif

#ifndef MAX_HEIGHT_MAPS
#define MAX_HEIGHT_MAPS 3
#endif

#ifndef MAX_SHADOW_MAPS
#define MAX_SHADOW_MAPS 2
#endif

#define TEXTURE_MAPS_NUM 312
#define SHADOW_MAPS_NUM 2

#define EPSILON 0.0001f
#define F0_COEFF 0.16f

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

struct DirectionalLight
{
	float3 Direction;
	float pad1; // Padding to align with HLSL's float4
	float3 Intensity;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float4x4 Transform;
};

#ifndef HLSL
}
#endif
