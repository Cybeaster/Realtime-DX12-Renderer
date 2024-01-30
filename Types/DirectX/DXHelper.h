#pragma once
#include "../../Utils/Statics.h"
#include "Types.h"

#include <dxgi1_3.h>
#include <dxgidebug.h>
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>

#include <unordered_map>
using namespace Microsoft::WRL;

// DirectX 12 specific headers.
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

// D3D12 extension library.
#include <d3dx12.h>

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <map>
#include <memory>

// Helper functions
#include "DirectXColors.h"
#include "Exception.h"

#include <directxcollision.h>

inline void ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();
}

struct SSubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
	DirectX::BoundingBox Bounds;
};

struct SMeshGeometry
{
	string Name;
	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	SSubmeshGeometry& FindSubmeshGeomentry(const string& SubmeshName)
	{
		if (!DrawArgs.contains(SubmeshName))
		{
			throw std::runtime_error("Submesh not found!");
		}
		return DrawArgs.at(SubmeshName);
	}

	SSubmeshGeometry& SetGeometry(const string& SubmeshName, const SSubmeshGeometry& Geometry)
	{
		if (DrawArgs.contains(SubmeshName))
		{
			throw std::runtime_error("Submesh is already exists!");
		}
		DrawArgs[SubmeshName] = Geometry;
		return DrawArgs.at(SubmeshName);
	}

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;
		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;
		return ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}

private:
	std::unordered_map<string, SSubmeshGeometry> DrawArgs;
};