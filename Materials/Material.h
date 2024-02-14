#pragma once

#include "DirectX/RenderConstants.h"
#include "MaterialData.h"

struct STexture;
struct SMaterial
{
	string Name;
	wstring TexturePath = L"";
	STexture* DiffuseTexture = nullptr;
	// Index into constant buffer corresponding to this material.
	int32_t MaterialCBIndex = -1;

	UINT NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to
	// update the constant buffer. Because we have a material constant
	// buffer for each FrameResource, we have to apply the update to each
	// FrameResource. Thus, when we modify a material we should set
	// NumFramesDirty = gNumFrameResources so that each frame resource
	// gets the update.

	int NumFramesDirty = SRenderConstants::NumFrameResources;

	// Material constant buffer data used for shading.
	SMaterialSurface MaterialSurface;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();
};

struct SMaterialDisplacementParams
{
	SMaterial* Material = nullptr;
	DirectX::XMFLOAT2 DisplacementMapTexelSize = { 1, 1 };
	float GridSpatialStep = 1.0f;
};
