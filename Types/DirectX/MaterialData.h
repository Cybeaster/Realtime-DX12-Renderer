#pragma once
#include "../../Utils/DirectXUtils.h"
#include "../../Utils/Math.h"
#include "RenderConstants.h"

struct SMaterialSurface
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
};

struct SMaterialData
{
	SMaterialSurface MaterialSurface;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();
	uint32_t DiffuseMapCount;
	uint32_t NormalMapCount;
	uint32_t HeightMapCount;
	uint32_t DiffuseMapIndex[SRenderConstants::MaxDiffuseMapsPerMaterial];
	uint32_t NormalMapIndex[SRenderConstants::MaxNormalMapsPerMaterial];
	uint32_t HeightMapIndex[SRenderConstants::MaxHeightMapsPerMaterial];
};
