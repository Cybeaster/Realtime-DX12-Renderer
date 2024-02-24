#pragma once
#include "Engine/RenderObject/RenderObject.h"
#include "Windows.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct STextureViewType
{
	static D3D12_SRV_DIMENSION GetViewType(const string& Type)
	{
		if (Type == Texture2D)
		{
			return D3D12_SRV_DIMENSION_TEXTURE2D;
		}
		if (Type == TextureCube)
		{
			return D3D12_SRV_DIMENSION_TEXTURECUBE;
		}
		return D3D12_SRV_DIMENSION_UNKNOWN;
	}

	static string GetViewString(const D3D12_SRV_DIMENSION Type)
	{
		if (Type == D3D12_SRV_DIMENSION_TEXTURE2D)
		{
			return Texture2D;
		}
		if (Type == D3D12_SRV_DIMENSION_TEXTURECUBE)
		{
			return TextureCube;
		}
		return "Unknown";
	}

	RENDER_TYPE(Texture2D);
	RENDER_TYPE(TextureCube);
};

struct STexture
{
	virtual ~STexture() = default;
	string Name;
	wstring FileName;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
	string ViewType = STextureViewType::Texture2D;
	int64_t HeapIdx = -1;

	virtual D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = Resource->GetDesc().Format;
		srvDesc.ViewDimension = STextureViewType::GetViewType(ViewType);
		srvDesc.Texture2D.MipLevels = Resource->GetDesc().MipLevels;
		return srvDesc;
	}
};
