#pragma once
#include "MathUtils.h"
#include "RenderConstants.h"

struct SMaterialSurface
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	DirectX::XMFLOAT3 Emission = { 0.0f, 0.0f, 0.0f };

	float Roughness = 0.25f;
	float IndexOfRefraction;
	float Dissolve = 1.0f;
};

/*
* real_t ambient[3];
  real_t diffuse[3];
  real_t specular[3];
  real_t transmittance[3];
  real_t emission[3];
  real_t shininess;
  real_t ior;       // index of refraction
  real_t dissolve;  // 1 == opaque; 0 == fully transparent
 */

struct SMaterialData
{
	SMaterialSurface MaterialSurface;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();
	UINT DiffuseMapCount;
	UINT NormalMapCount;
	UINT HeightMapCount;
	UINT DiffuseMapIndex[SRenderConstants::MaxDiffuseMapsPerMaterial];
	UINT NormalMapIndex[SRenderConstants::MaxNormalMapsPerMaterial];
	UINT HeightMapIndex[SRenderConstants::MaxHeightMapsPerMaterial];
	float pad1;
	float pad2;
};
