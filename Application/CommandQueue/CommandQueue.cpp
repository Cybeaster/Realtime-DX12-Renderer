#include "CommandQueue.h"

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
	CHECK(FenceEvent, "Failed to create fence event handle.");

	CommandList->Close();
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

uint64_t OCommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList)
{
	CommandList->Close();

	ID3D12CommandAllocator* allocator;
	UINT dataSize = sizeof(allocator);

	THROW_IF_FAILED(CommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &allocator));

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
	THROW_IF_FAILED(CommandQueue->Signal(Fence.Get(), fenceValueForSignal));

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

void OCommandQueue::ResetCommandList()
{
	CommandList->Reset(CommandAllocator.Get(), nullptr);
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
