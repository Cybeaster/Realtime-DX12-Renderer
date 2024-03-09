#pragma once
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "InstanceData.h"
#include "Logger.h"
#include "MaterialData.h"
#include "ObjectConstants.h"
#include "Vertex.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct SFrameResource
{
	SFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount, IRenderObject* Owner);

	SFrameResource(const SFrameResource&) = delete;

	SFrameResource& operator=(const SFrameResource&) = delete;

	~SFrameResource();

	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// We cannot update a cbuffer until the GPU is done processing the
	// commands that reference it. So each frame needs their own cbuffers.

	unique_ptr<OUploadBuffer<SPassConstants>> PassCB = nullptr;
	unique_ptr<OUploadBuffer<SMaterialData>> MaterialBuffer = nullptr;
	std::unique_ptr<OUploadBuffer<SInstanceData>> InstanceBuffer = nullptr;

	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
	IRenderObject* Owner = nullptr;
};

inline SFrameResource::SFrameResource(ID3D12Device* Device, UINT PassCount, UINT MaxInstanceCount, UINT MaterialCount, IRenderObject* Owner)
    : Owner(Owner)
{
	THROW_IF_FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	if (MaxInstanceCount == 0)
	{
		LOG(Engine, Warning, "MaxInstanceCount is 0");
		return;
	}
	PassCB = make_unique<OUploadBuffer<SPassConstants>>(Device, PassCount, true, Owner);
	InstanceBuffer = make_unique<OUploadBuffer<SInstanceData>>(Device, MaxInstanceCount, false, Owner);
	if (MaterialCount > 0)
	{
		MaterialBuffer = make_unique<OUploadBuffer<SMaterialData>>(Device, MaterialCount, false, Owner);
	}
	else
	{
		LOG(Engine, Warning, "Material count is 0");
	}
}

inline SFrameResource::~SFrameResource()
{
}
