#pragma once
#include <Types.h>

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>

// DirectX 12 specific headers.
#include <d3d12.h>


class OCommandQueue
{
public:
	OCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> Device, D3D12_COMMAND_LIST_TYPE Type);

	virtual ~OCommandQueue();

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList);

	uint64_t Signal();

	bool IsFenceComplete(uint64_t FenceValue);

	void WaitForFenceValue(uint64_t FenceValue);

	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue();

protected:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator);

private:
	struct CommandAllocatorEntry
	{
		uint64_t FenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	};

	using TCommandAllocatorQueue = queue<CommandAllocatorEntry>;
	using TCommandListQueue = queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

	D3D12_COMMAND_LIST_TYPE CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	HANDLE FenceEvent;
	uint64_t FenceValue;

	TCommandAllocatorQueue CommandAllocatorQueue;
	TCommandListQueue CommandListQueue;
};



