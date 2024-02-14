#pragma once
#include <DirectX/DXHelper.h>
#include <Types.h>
class OCommandQueue
{
public:
	OCommandQueue(ComPtr<ID3D12Device2> Device, D3D12_COMMAND_LIST_TYPE Type);
	virtual ~OCommandQueue();

	ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	ComPtr<ID3D12CommandAllocator> GetCommandAllocator();
	ComPtr<ID3D12CommandQueue> GetCommandQueue();

	uint64_t ExecuteCommandList();
	void ExecuteCommandListAndWait();

	uint64_t Signal();

	bool IsFenceComplete(uint64_t FenceValue) const;
	void WaitForFenceValue(uint64_t FenceValue);
	void Flush();
	void TryResetCommandList();
	ComPtr<ID3D12Fence> GetFence() const;

protected:
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> Allocator);

private:
	struct CommandAllocatorEntry
	{
		uint64_t FenceValue;
		ComPtr<ID3D12CommandAllocator> CommandAllocator;
	};

	using TCommandAllocatorQueue = queue<CommandAllocatorEntry>;
	using TCommandListQueue = queue<ComPtr<ID3D12GraphicsCommandList>>;

	D3D12_COMMAND_LIST_TYPE CommandListType;
	ComPtr<ID3D12Device2> Device = nullptr;
	ComPtr<ID3D12CommandQueue> CommandQueue = nullptr;
	ComPtr<ID3D12Fence> Fence = nullptr;
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> CommandList;

	HANDLE FenceEvent;
	uint64_t FenceValue;

	TCommandAllocatorQueue CommandAllocatorQueue;
	TCommandListQueue CommandListQueue;

	bool IsReset = false;
};
