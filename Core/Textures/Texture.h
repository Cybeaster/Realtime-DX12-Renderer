#pragma once
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "Windows.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct STextureViewType
{
	static D3D12_SRV_DIMENSION GetViewType(const string& Type);

	static string GetViewString(const D3D12_SRV_DIMENSION Type);
	static vector<string> GetTextureTypes();

	RENDER_TYPE(Texture2D);
	RENDER_TYPE(TextureCube);
};

ENUM(ETextureType, Diffuse, Normal, Height, Occlusion, Roughness)

inline wstring ToString(ETextureType Type)
{
	switch (Type)
	{
	case ETextureType::Diffuse:
		return L"Diffuse";
	case ETextureType::Normal:
		return L"Normal";
	case ETextureType::Height:
		return L"Height";
	case ETextureType::Occlusion:
		return L"Occlusion";
	case ETextureType::Roughness:
		return L"Roughness";
	default:
		return L"Unknown";
	}
}
struct STexture
{
	virtual ~STexture() = default;
	string Name;
	wstring FileName;

	SResourceInfo Resource;
	SResourceInfo UploadHeap;
	string ViewType = STextureViewType::Texture2D;
	//Index for accessing the texture in the local array in shader
	int64_t TextureIndex = -1;
	SDescriptorPair SRV;
	ETextureType Type = ETextureType::Diffuse;
	virtual D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const;
};
