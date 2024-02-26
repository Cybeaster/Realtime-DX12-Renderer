#pragma once
#include "../../Utils/DirectXUtils.h"
#include "../../Utils/Math.h"
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

	UINT DiffuseMapIndex = 0;
	int NormalMapIndex = 0;
	UINT MaterialPad1;
	UINT MaterialPad2;
};
