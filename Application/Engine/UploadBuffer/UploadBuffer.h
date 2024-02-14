#pragma once
#include "..\..\..\Utils\DirectXUtils.h"
#include "Exception.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

template<typename Type>
class OUploadBuffer
{
public:
	OUploadBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer);

	OUploadBuffer(const OUploadBuffer&) = delete;

	OUploadBuffer& operator=(const OUploadBuffer&) = delete;

	~OUploadBuffer()
	{
		if (UploadBuffer != nullptr)
		{
			UploadBuffer->Unmap(0, nullptr);
		}
		MappedData = nullptr;
	}

	ID3D12Resource* GetResource() const
	{
		return UploadBuffer.Get();
	}

	void CopyData(int ElementIdx, const Type& Data)
	{
		memcpy(&MappedData[ElementIdx * ElementByteSize], &Data, sizeof(Type));
	}

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer;
	UINT ElementByteSize = 0;
	bool bIsConstantBuffer = false;
	BYTE* MappedData = nullptr;
};

template<typename Type>
OUploadBuffer<Type>::OUploadBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer)
	: bIsConstantBuffer(IsConstantBuffer)
{
	ElementByteSize = sizeof(Type);

	// Constant buffer elements need to be multiples of 256 bytes.
	// This is because the hardware can only view constant data
	// at m*256 byte offsets and of n*256 byte lengths.
	// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
	// UINT64 OffsetInBytes; // multiple of 256
	// UINT SizeInBytes; // multiple of 256
	// } D3D12_CONSTANT_BUFFER_VIEW_DESC;

	if (IsConstantBuffer)
	{
		ElementByteSize = Utils::CalcBufferByteSize(sizeof(Type));
	}
	const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * ElementCount);
	THROW_IF_FAILED(Device->CreateCommittedResource(
		&uploadHeap, // Upload heap
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc, // Resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&UploadBuffer)));

	THROW_IF_FAILED(UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));
}
