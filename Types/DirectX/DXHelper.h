#pragma once

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

#define RENDER_TYPE(type) inline static const string type = #type

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
	std::string Name;
	std::unique_ptr<std::vector<DirectX::XMFLOAT3>> Vertices = nullptr;
	std::unique_ptr<std::vector<uint32_t>> Indices = nullptr;
};

struct SMeshGeometry
{
	std::string Name;
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

	SSubmeshGeometry* FindSubmeshGeomentry(const std::string& SubmeshName)
	{
		if (!DrawArgs.contains(SubmeshName))
		{
			throw std::runtime_error("Submesh not found!");
		}
		return &DrawArgs.at(SubmeshName);
	}

	SSubmeshGeometry& SetGeometry(const std::string& SubmeshName, SSubmeshGeometry& Geometry)
	{
		if (DrawArgs.contains(SubmeshName))
		{
			throw std::runtime_error("Submesh is already exists!");
		}
		DrawArgs[SubmeshName] = std::move(Geometry);
		return DrawArgs.at(SubmeshName);
	}

	const std::unordered_map<std::string, SSubmeshGeometry>& GetDrawArgs()
	{
		return DrawArgs;
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

	std::unordered_map<std::string, SSubmeshGeometry> DrawArgs;
};

inline bool IsKeyPressed(const char Key)
{
	return GetAsyncKeyState(Key) & 0x8000;
}

inline std::string WStringToUTF8(const std::wstring& Wstr)
{
	if (Wstr.empty())
	{
		return std::string();
	}
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &Wstr[0], static_cast<int>(Wstr.size()), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &Wstr[0], static_cast<int>(Wstr.size()), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}

inline std::wstring UTF8ToWString(const std::string& Str)
{
	if (Str.empty())
	{
		return std::wstring();
	}
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &Str[0], static_cast<int>(Str.size()), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &Str[0], static_cast<int>(Str.size()), &wstrTo[0], size_needed);
	return wstrTo;
}