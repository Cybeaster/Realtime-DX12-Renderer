#pragma once
#include "Engine/RenderObject/RenderObject.h"
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

struct STexture
{
	virtual ~STexture() = default;
	string Name;
	wstring FileName;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
	string ViewType = STextureViewType::Texture2D;
	int64_t HeapIdx = -1;

	virtual D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const;
};
