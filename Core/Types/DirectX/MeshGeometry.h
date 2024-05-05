#pragma once

#include "DXHelper.h"
#include "Logger.h"
#include "Material.h"

struct SSubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;
	DirectX::BoundingBox Bounds;
	std::string Name;
	std::unique_ptr<std::vector<DirectX::XMFLOAT3>> Vertices = nullptr;
	std::unique_ptr<std::vector<uint32_t>> Indices = nullptr;
	SMaterial* Material = nullptr;
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
		string name = SubmeshName;
		if (DrawArgs.contains(SubmeshName))
		{
			LOG(Render, Warning, "Submesh already exists!");
			name += "_" + std::to_string(DrawArgs.size());
		}
		DrawArgs[name] = std::move(Geometry);
		return DrawArgs.at(name);
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