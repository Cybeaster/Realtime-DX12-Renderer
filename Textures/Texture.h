#pragma once
#include "Windows.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct STexture
{
	string Name;
	wstring FileName;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;

	int64_t HeapIdx = -1;
};
