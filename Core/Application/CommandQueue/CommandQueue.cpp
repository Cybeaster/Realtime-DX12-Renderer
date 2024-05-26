#include "CommandQueue.h"

#include "DirectX/ShaderTypes.h"
#include "Logger.h"

#include <Exception.h>

OCommandQueue::OCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> Device, D3D12_COMMAND_LIST_TYPE Type)
    : FenceValue(0)
    , CommandListType(Type)
    , Device(Device)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = Type;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	THROW_IF_FAILED(Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&CommandQueue)));
	THROW_IF_FAILED(Device->CreateCommandAllocator(Type, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));
	THROW_IF_FAILED(Device->CreateCommandList(0, Type, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(CommandList.GetAddressOf())));

	FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	CHECK(FenceEvent);

	CommandList->Close();
	IsReset = false;
	THROW_IF_FAILED(Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
}

OCommandQueue::~OCommandQueue()
{
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> OCommandQueue::GetCommandList()
{
	return CommandList;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> OCommandQueue::GetCommandAllocator()
{
	return CommandAllocator;
}

uint64_t OCommandQueue::ExecuteCommandList()
{
	if (!IsReset)
	{
		LOG(Engine, Warning, "Command list has to be reset before closing!");
		return 0;
	}

	IsReset = false;
	CommandList->Close();
	ID3D12CommandList* const commandLists[] = {
		CommandList.Get()
	};

	CommandQueue->ExecuteCommandLists(1, commandLists);
	uint64_t fenceValue = Signal();
	return fenceValue;
}

void OCommandQueue::ExecuteCommandListAndWait()
{
	WaitForFenceValue(ExecuteCommandList());
}

/*
 * Purpose: It's used to indicate a point in time in the command queue and is commonly used for synchronization purposes.
*    The GPU processes commands in the order they are placed in the command queue.
*    By placing a signal command in the command queue,
*    you ensure that the GPU updates the fence's value only after all preceding commands have been completed.
*    This is crucial for synchronization.
*
    If the fence update were done outside the command queue, there would be no guarantee about when it happens relative to other GPU operations.
    It might signal completion before the actual work is done, leading to incorrect program behavior.
 */

uint64_t OCommandQueue::Signal()
{
	uint64_t fenceValueForSignal = ++FenceValue;
	// Put the fence in the command queue, exlicitly telling up to what point we consider those command to get done.
	THROW_IF_FAILED(CommandQueue->Signal(Fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

/*
 *     Purpose: Ensures that the CPU waits for the GPU to reach a certain point in execution.
 */
void OCommandQueue::WaitForFenceValue(uint64_t FenceValue)
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		FenceEvent = ::CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		THROW_IF_FAILED(Fence->SetEventOnCompletion(FenceValue, FenceEvent));
		::WaitForSingleObject(FenceEvent, INFINITE);
		::CloseHandle(FenceEvent);
	}
}

/*
 * This function is a combination of the previous two, signaling a fence and then waiting for its completion.
 * Purpose: Used to ensure that all previously submitted commands on a command queue are completed before the CPU proceeds.
 */
void OCommandQueue::Flush()
{
	WaitForFenceValue(Signal());
}

void OCommandQueue::TryResetCommandList()
{
	if (IsReset)
	{
		LOG(Engine, Warning, "Command list is already reset!");
		return;
	}
	IsReset = true;
	THROW_IF_FAILED(CommandList->Reset(CommandAllocator.Get(), nullptr));
}

Microsoft::WRL::ComPtr<ID3D12Fence> OCommandQueue::GetFence() const
{
	return Fence;
}

void OCommandQueue::SetPipelineState(SPSODescriptionBase* PSOInfo)
{
	if (CurrentPSO == PSOInfo || PSOInfo == nullptr)
	{
		return;
	}

	CurrentPSO = PSOInfo;
	SetResources.clear();
	LOG(Engine, Log, "Setting pipeline state for PSO: {}", TEXT(PSOInfo->Name));
	CommandList->SetPipelineState(PSOInfo->PSO.Get());

	if (CurrentPSO->Type == EPSOType::Graphics)
	{
		CommandList->SetGraphicsRootSignature(PSOInfo->RootSignature->RootSignatureParams.RootSignature.Get());
	}
	else
	{
		CommandList->SetComputeRootSignature(PSOInfo->RootSignature->RootSignatureParams.RootSignature.Get());
	}
}

void OCommandQueue::SetResource(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Resource, SPSODescriptionBase* PSO)
{
	if (CurrentPSO != PSO)
	{
		LOG(Engine, Warning, "Trying to set resource view for a different PSO!")
		SetPipelineState(PSO);
	}

	if (PSO == nullptr)
	{
		LOG(Engine, Warning, "PSO is nullptr!")
		return;
	}

	if (SetResources.contains(Name) && SetResources[Name] == Resource)
	{
		LOG(Engine, Warning, "Resource {} already set!", TEXT(Name));
		return;
	}

	PSO->RootSignature->SetResource(Name, Resource, CommandList.Get());
	SetResources[Name] = Resource;
}

void OCommandQueue::SetResource(const string& Name, D3D12_GPU_DESCRIPTOR_HANDLE Resource, SPSODescriptionBase* PSO)
{
	if (CurrentPSO != PSO)
	{
		LOG(Engine, Warning, "Trying to set resource view for a different PSO!")
		SetPipelineState(PSO);
	}

	if (PSO == nullptr)
	{
		LOG(Engine, Warning, "PSO is nullptr!")
		return;
	}

	if (SetResources.contains(Name) && SetResources[Name] == Resource.ptr)
	{
		LOG(Engine, Warning, "Resource {} already set!", TEXT(Name));
		return;
	}

	PSO->RootSignature->SetResource(Name, Resource, CommandList.Get());
	SetResources[Name] = Resource.ptr;
}

D3D12_RESOURCE_STATES OCommandQueue::ResourceBarrier(ORenderTargetBase* Resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter) const
{
	return Utils::ResourceBarrier(CommandList.Get(), Resource->GetResource(), StateBefore, StateAfter);
}

D3D12_RESOURCE_STATES OCommandQueue::ResourceBarrier(ORenderTargetBase* Resource, D3D12_RESOURCE_STATES StateAfter) const
{
	return Utils::ResourceBarrier(CommandList.Get(), Resource->GetResource(), StateAfter);
}

D3D12_RESOURCE_STATES OCommandQueue::ResourceBarrier(SResourceInfo* Resource, D3D12_RESOURCE_STATES StateAfter) const
{
	return Utils::ResourceBarrier(CommandList.Get(), Resource, StateAfter);
}

void OCommandQueue::CopyResourceTo(ORenderTargetBase* Dest, ORenderTargetBase* Src) const
{
	if (Dest == Src)
	{
		LOG(Engine, Error, "Source and destination are the same!");
		return;
	}
	CopyResourceTo(Dest->GetResource(), Src->GetResource());
}

void OCommandQueue::CopyResourceTo(SResourceInfo* Dest, SResourceInfo* Src) const
{
	if (Dest == Src)
	{
		LOG(Engine, Error, "Source and destination are the same!");
		return;
	}
	auto destOld = ResourceBarrier(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	auto srcOld = ResourceBarrier(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	CommandList->CopyResource(Dest->Resource.Get(), Src->Resource.Get());
	ResourceBarrier(Dest, destOld);
	ResourceBarrier(Src, srcOld);
}

ORenderTargetBase* OCommandQueue::SetRenderTarget(ORenderTargetBase* RenderTarget, uint32_t Subtarget)
{
	return SetRenderTargetImpl(RenderTarget, false, false, Subtarget);
}

ORenderTargetBase* OCommandQueue::SetRenderTargetImpl(ORenderTargetBase* RenderTarget, bool ClearDepth, bool ClearRenderTarget, uint32_t Subtarget)
{
	if (CurrentRenderTarget && CurrentRenderTarget != RenderTarget)
	{
		CurrentRenderTarget->UnsetRenderTarget(this);
	}

	RenderTarget->PrepareRenderTarget(this, ClearRenderTarget, ClearDepth, Subtarget);
	CurrentRenderTarget = RenderTarget;
	return RenderTarget;
}

ORenderTargetBase* OCommandQueue::SetAndClearRenderTarget(ORenderTargetBase* RenderTarget, uint32_t Subtarget)
{
	return SetRenderTargetImpl(RenderTarget, true, true, Subtarget);
}

void OCommandQueue::ClearRenderTarget(const SDescriptorPair& RTV, const SColor Color) const
{
	const auto floatColor = Color.ToFloat4();
	FLOAT color[4] = {
		floatColor.x,
		floatColor.y,
		floatColor.z,
		floatColor.w
	};
	auto resource = RTV.Resource.lock();
	LOG(Render, Log, "Clearing render target: of {} with index {}", TEXT(resource.get()), TEXT(RTV.Index));
	CommandList->ClearRenderTargetView(RTV.CPUHandle, color, 0, nullptr);
}

void OCommandQueue::ClearDepthStencil(const SDescriptorPair& DSV) const
{
	LOG(Render, Log, "Clearing depth stencil of {} with index", TEXT(DSV.Resource.lock().get()), TEXT(DSV.Index));
	auto resource = DSV.Resource.lock();
	if (resource->CurrentState != D3D12_RESOURCE_STATE_DEPTH_WRITE)
	{
		LOG(Render, Error, "Depth stencil resource is not in the correct state!");
		ResourceBarrier(resource.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	CommandList->ClearDepthStencilView(DSV.CPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void OCommandQueue::SetRenderTargets(const SDescriptorPair& RTV, const SDescriptorPair& DSV) const
{
	auto dsvResource = DSV.Resource.lock();
	auto rtvResource = RTV.Resource.lock();
	LOG(Render, Log, "Setting render target: {} and depth stencil: {}", TEXT(rtvResource.get()), TEXT(dsvResource.get()));
	ResourceBarrier(dsvResource.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	ResourceBarrier(rtvResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->OMSetRenderTargets(1, &RTV.CPUHandle, true, &DSV.CPUHandle);
}

void OCommandQueue::SetRenderToRTVOnly(const SDescriptorPair& RTV) const
{
	LOG(Render, Log, "Setting only render target: {}", TEXT(RTV.Resource.lock().get()));
	auto rtvResource = RTV.Resource.lock();
	ResourceBarrier(rtvResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->OMSetRenderTargets(1, &RTV.CPUHandle, true, nullptr);
}

void OCommandQueue::SetRenderToDSVOnly(const SDescriptorPair& DSV) const
{
	LOG(Render, Log, "Setting only depth stencil: {}", TEXT(DSV.Resource.lock().get()));
	auto dsvResource = DSV.Resource.lock();
	ResourceBarrier(dsvResource.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	CommandList->OMSetRenderTargets(0, nullptr, true, &DSV.CPUHandle);
}

void OCommandQueue::ResetQueueState()
{
	CurrentRenderTarget->UnsetRenderTarget(this);
	CurrentRenderTarget = nullptr;
	CurrentPSO = nullptr;
	SetResources.clear();
	CurrentObjectHeap = nullptr;
}

void OCommandQueue::SetViewportScissors(const D3D12_VIEWPORT& Viewport, const D3D12_RECT& Scissors) const
{
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &Scissors);
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> OCommandQueue::GetCommandQueue()
{
	return CommandQueue;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> OCommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	THROW_IF_FAILED(Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(&allocator)));
	return allocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> OCommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> list;
	THROW_IF_FAILED(Device->CreateCommandList(0, CommandListType, Allocator.Get(), nullptr, IID_PPV_ARGS(&list)));
	return list;
}

void OCommandQueue::SetHeap(SRenderObjectHeap* Heap)
{
	if (Heap == CurrentObjectHeap)
	{
		LOG(Engine, Warning, "Heap is already set!")
		return;
	}
	LOG(Engine, Warning, "Setting heap: {}", TEXT(Heap->SRVHeap.Get()));
	CurrentObjectHeap = Heap;
	ID3D12DescriptorHeap* heaps[] = { Heap->SRVHeap.Get() };
	GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
}
