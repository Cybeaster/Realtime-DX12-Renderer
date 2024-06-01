#pragma once

#include "../Textures/Texture.h"
#include "DirectX/RenderConstants.h"
#include "LightComponent/LightComponent.h"

struct STexture;

struct STexturePath
{
	bool IsValid() const
	{
		return Texture != nullptr && !Path.empty();
	}

	STexture* Texture = nullptr;
	wstring Path;
};

struct SMaterialPayloadData
{
	string Name;
	HLSL::MaterialData MaterialSurface;
	wstring NormalMap;
	wstring DiffuseMap;
	wstring HeightMap;
	wstring AlphaMap;
	wstring AmbientMap;
	wstring SpecularMap;
};

struct SMaterial
{
	string Name;
	STexturePath DiffuseMap;
	STexturePath NormalMap;
	STexturePath HeightMap;
	STexturePath AlphaMap;
	STexturePath AmbientMap;
	STexturePath SpecularMap;
	SRenderLayer RenderLayer = SRenderLayers::Opaque;

	// Index into constant buffer corresponding to this material.
	int32_t MaterialCBIndex = -1;

	// Dirty flag indicating the material has changed and we need to
	// update the constant buffer. Because we have a material constant
	// buffer for each FrameResource, we have to apply the update to each
	// FrameResource. Thus, when we modify a material we should set
	// NumFramesDirty = gNumFrameResources so that each frame resource
	// gets the update.

	int NumFramesDirty = SRenderConstants::NumFrameResources;

	// Material constant buffer data used for shading.
	HLSL::MaterialData MaterialData;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();

	SDelegate<void> OnMaterialChanged;

private:
	static vector<uint32_t> GetTextureIndices(const vector<STexturePath>& Textures);
};
