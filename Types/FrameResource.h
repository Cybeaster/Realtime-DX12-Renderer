#pragma once
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "ObjectConstants.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>
struct SFrameResource
{
	SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount);
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

	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
};

inline SFrameResource::SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount)
{
	THROW_IF_FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	PassCB = make_unique<OUploadBuffer<SPassConstants>>(Device, PassCount, true);
	ObjectCB = make_unique<OUploadBuffer<SObjectConstants>>(Device, ObjectCount, true);
}

inline SFrameResource::~SFrameResource()
{
}