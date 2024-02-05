#pragma once
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Logger.h"
#include "MaterialData.h"
#include "ObjectConstants.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct SVertex
{
	SVertex() = default;
	SVertex(float X, float Y, float Z, float Nx, float Ny, float Nz, float U, float V)
	    : Pos(X, Y, Z)
	    , Normal(Nx, Ny, Nz)
	    , TexC(U, V)
	{
	}

	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
};

struct SFrameResource
{
	SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount);

	SFrameResource(const SFrameResource&) = delete;

	SFrameResource& operator=(const SFrameResource&) = delete;

	~SFrameResource();

	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// We cannot update a cbuffer until the GPU is done processing the
	// commands that reference it. So each frame needs their own cbuffers.

	unique_ptr<OUploadBuffer<SPassConstants>> PassCB = nullptr;
	unique_ptr<OUploadBuffer<SObjectConstants>> ObjectCB = nullptr;
	unique_ptr<OUploadBuffer<SMaterialData>> MaterialBuffer = nullptr;

	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
};

inline SFrameResource::SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount)
{
	THROW_IF_FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	PassCB = make_unique<OUploadBuffer<SPassConstants>>(Device, PassCount, true);
	ObjectCB = make_unique<OUploadBuffer<SObjectConstants>>(Device, ObjectCount, true);
	if (MaterialCount > 0)
	{
		MaterialBuffer = make_unique<OUploadBuffer<SMaterialData>>(Device, MaterialCount, true);
	}
	else
	{
		LOG(Engine, Warning, "Material count is 0");
	}
}

inline SFrameResource::~SFrameResource()
{
}
