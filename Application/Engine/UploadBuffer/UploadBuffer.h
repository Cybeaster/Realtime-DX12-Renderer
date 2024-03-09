#pragma once

#include "DirectX/Resource.h"
#include "DirectXUtils.h"

template<typename Type>
class OUploadBuffer
{
public:
	OUploadBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer, IRenderObject* Owner = nullptr);

	OUploadBuffer(const OUploadBuffer&) = delete;

	OUploadBuffer& operator=(const OUploadBuffer&) = delete;

	~OUploadBuffer()
	{
		if (UploadBuffer.Resource != nullptr)
		{
			UploadBuffer.Resource->Unmap(0, nullptr);
		}
		MappedData = nullptr;
	}

	SResourceInfo* GetResource()
	{
		return &UploadBuffer;
	}

	void CopyData(int ElementIdx, const Type& Data)
	{
		memcpy(&MappedData[ElementIdx * ElementByteSize], &Data, sizeof(Type));
	}

public:
	SResourceInfo UploadBuffer;
	IRenderObject* Owner = nullptr;
	UINT ElementByteSize = 0;
	bool bIsConstantBuffer = false;
	BYTE* MappedData = nullptr;
};

template<typename Type>
OUploadBuffer<Type>::OUploadBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstantBuffer, IRenderObject* Owner)
    : bIsConstantBuffer(IsConstantBuffer), Owner(Owner)
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
	const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * ElementCount);

	UploadBuffer = Utils::CreateResource(Owner, Device, D3D12_HEAP_TYPE_UPLOAD, uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
	THROW_IF_FAILED(UploadBuffer.Resource->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));
}
