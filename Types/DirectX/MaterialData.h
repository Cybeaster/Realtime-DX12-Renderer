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
	float IndexOfRefraction;
	float Dissolve = 1.0f;
	int32_t Illumination = 0.0f;
	float Roughness = 0.25f;
	float Metallic = 0.0f;
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
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();

	UINT DiffuseMapCount = 0;
	UINT NormalMapCount = 0;
	UINT HeightMapCount = 0;
	float pad0;

	UINT DiffuseMapIndex[SRenderConstants::MaxDiffuseMapsPerMaterial] = { 0, 0, 0 };
	float pad1 = 0;

	UINT NormalMapIndex[SRenderConstants::MaxNormalMapsPerMaterial] = { 0, 0, 0 };
	float pad01;

	UINT HeightMapIndex[SRenderConstants::MaxHeightMapsPerMaterial] = { 0, 0, 0 };
	STextureShaderData AlphaMap;
	STextureShaderData SpecularMap;
	STextureShaderData AmbientMap;

	float pad04 = 0;
	float pad05 = 0;
};
