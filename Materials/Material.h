#pragma once

#include "../Textures/Texture.h"
#include "DirectX/MaterialData.h"
#include "DirectX/RenderConstants.h"

struct STexture;

struct STexturePath
{
	STexture* Texture = nullptr;
	wstring Path;
};

struct SMaterial
{
	string Name;

	vector<STexturePath> NormalMaps;
	vector<STexturePath> DiffuseMaps;
	vector<STexturePath> HeightMaps;

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
	SMaterialSurface MaterialSurface;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = Utils::Math::Identity4x4();

	vector<uint32_t> GetDiffuseMapIndices() const;
	vector<uint32_t> GetNormalMapIndices() const;
	vector<uint32_t> GetHeightMapIndices() const;

private:
	static vector<uint32_t> GetTextureIndices(const vector<STexturePath>& Textures);
};

inline vector<uint32_t> SMaterial::GetDiffuseMapIndices() const
{
	return GetTextureIndices(DiffuseMaps);
}

inline vector<uint32_t> SMaterial::GetNormalMapIndices() const
{
	return GetTextureIndices(NormalMaps);
}

inline vector<uint32_t> SMaterial::GetHeightMapIndices() const
{
	return GetTextureIndices(HeightMaps);
}

inline vector<uint32_t> SMaterial::GetTextureIndices(const vector<STexturePath>& Textures)
{
	vector<uint32_t> indices;
	for (auto [texture, path] : Textures)
	{
		if (texture)
		{
			indices.push_back(texture->HeapIdx);
		}
	}
	return indices;
}

struct SMaterialParams
{
	SMaterialParams() = default;
	SMaterialParams(SMaterial* Mat)
	    : Material(Mat) {}

	SMaterial* Material = nullptr;
	DirectX::XMFLOAT2 DisplacementMapTexelSize = { 1, 1 };
	float GridSpatialStep = 1.0f;
};
