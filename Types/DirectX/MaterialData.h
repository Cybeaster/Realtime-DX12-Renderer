#pragma once
#include "MathUtils.h"
#include "RenderConstants.h"

struct SMaterialSurface
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float pad1;

	DirectX::XMFLOAT3 Emission = { 0.0f, 0.0f, 0.0f };
	float pad2;

	float Roughness = 0.25f;
	float IndexOfRefraction;
	float Dissolve = 1.0f;
	float pad3;
};

struct SMaterialData
{
	SMaterialSurface MaterialSurface;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();
	UINT DiffuseMapCount = 0;
	UINT NormalMapCount = 0;
	UINT HeightMapCount = 0;
	UINT DiffuseMapIndex[SRenderConstants::MaxDiffuseMapsPerMaterial] = { 0, 0, 0 };
	UINT NormalMapIndex[SRenderConstants::MaxNormalMapsPerMaterial] = { 0, 0, 0 };
	UINT HeightMapIndex[SRenderConstants::MaxHeightMapsPerMaterial] = { 0, 0, 0 };
	float pad1 = 0;
	float pad2 = 0;
};
