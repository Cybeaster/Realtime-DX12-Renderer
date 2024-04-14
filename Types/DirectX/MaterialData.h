#pragma once
#include "MathUtils.h"
#include "RenderConstants.h"

struct STexturePath;
struct SMaterialSurface
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };

	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	DirectX::XMFLOAT3 Transmittance = { 0.0f, 0.0f, 0.0f };
	float IndexOfRefraction;

	DirectX::XMFLOAT3 Emission = { 0.0f, 0.0f, 0.0f };
	float Dissolve = 1.0f;
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
