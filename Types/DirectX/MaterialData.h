#pragma once
#include "MathUtils.h"
#include "RenderConstants.h"

struct STexturePath;
struct SMaterialSurface
{
	DirectX::XMFLOAT3 AmbientAlbedo = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 SpecularAlbedo = { 0.5f, 0.5f, 0.5f };
	DirectX::XMFLOAT3 Transmittance = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Emission = { 0.0f, 0.0f, 0.0f };
	float Shininess = 1.0f;
	float IndexOfRefraction = 1.0f;
	float Dissolve = 1.0f;
	int32_t Illumination = 0.0f;
	float Roughness = 0.25f;
	float Metallness = 0.0f;
	float Sheen = 0.0f;
};

struct STextureShaderData
{
	UINT bIsEnabled = 0;
	UINT TextureIndex = 0;
};

struct SMaterialData
{
	SMaterialSurface MaterialSurface;
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();
	STextureShaderData DiffuseMap;
	STextureShaderData NormalMap;
	STextureShaderData HeightMap;
	STextureShaderData AlphaMap;
	STextureShaderData SpecularMap;
	STextureShaderData AmbientMap;
};
