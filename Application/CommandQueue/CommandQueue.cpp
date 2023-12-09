#include "CommandQueue.h"

#include <Exception.h>

OCommandQueue::OCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> Device, D3D12_COMMAND_LIST_TYPE Type)
    : FenceValue(0)
    , CommandListType(Type)
    , Device(Device)
{

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = Type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&CommandQueue)));
	ThrowIfFailed(Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

	CHECK(FenceEvent, "Failed to create fence event handle.");
}

OCommandQueue::~OCommandQueue()
{
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> OCommandQueue::GetCommandList()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> list;

	if (!CommandAllocatorQueue.empty() && IsFenceComplete(CommandAllocatorQueue.front().FenceValue))
	{
		allocator = CommandAllocatorQueue.front().CommandAllocator;
		CommandAllocatorQueue.pop();
		ThrowIfFailed(allocator->Reset());
	}
	else
	{
		allocator = CreateCommandAllocator();
	}

	if (!CommandListQueue.empty())
	{
		list = CommandListQueue.front();
		CommandListQueue.pop();
		ThrowIfFailed(list->Reset(allocator.Get(), nullptr));
	}
	else
	{
		list = CreateCommandList(allocator);
	}
	ThrowIfFailed(list->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), allocator.Get()));
	return list;
}

uint64_t OCommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList)
{
	CommandList->Close();

	ID3D12CommandAllocator* allocator;
	UINT dataSize = sizeof(allocator);

	ThrowIfFailed(CommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &allocator));

	ID3D12CommandList* const commandLists[] = {
		CommandList.Get()
	};

	CommandQueue->ExecuteCommandLists(1, commandLists);
	uint64_t fenceValue = Signal();
	CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, allocator });
	CommandListQueue.push(CommandList);
	allocator->Release();
	return fenceValue;
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
	ThrowIfFailed(CommandQueue->Signal(Fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

bool OCommandQueue::IsFenceComplete(uint64_t FenceValue) const
{
	return Fence->GetCompletedValue() >= FenceValue;
}

/*
 *     Purpose: Ensures that the CPU waits for the GPU to reach a certain point in execution.
 */
void OCommandQueue::WaitForFenceValue(uint64_t FenceValue)
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		ThrowIfFailed(Fence->SetEventOnCompletion(FenceValue, FenceEvent));
		::WaitForSingleObject(FenceEvent, DWORD_MAX);
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

Microsoft::WRL::ComPtr<ID3D12CommandQueue> OCommandQueue::GetCommandQueue()
{
	return CommandQueue;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> OCommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	ThrowIfFailed(Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(&allocator)));
	return allocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> OCommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> list;
	ThrowIfFailed(Device->CreateCommandList(0, CommandListType, Allocator.Get(), nullptr, IID_PPV_ARGS(&list)));
	return list;
}
