#pragma once
#include "../Types/DirectX/DXHelper.h"

struct IDescriptor
{
	virtual ~IDescriptor() = default;
};

struct SRenderObjectDescriptor : IDescriptor
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUSRVescriptor;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUSRVDescriptor;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPURTVDescriptor;

	UINT DSVSRVUAVDescriptorSize;
	UINT RTVDescriptorSize;

	void SRVCPUOffset(UINT Value)
	{
		CPUSRVescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
	}

	void SRVGPUOffset(UINT Value)
	{
		GPUSRVDescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
	}

	void RTVCPUOffset(UINT Value)
	{
		CPURTVDescriptor.Offset(Value, RTVDescriptorSize);
	}

	void OffsetSRV(UINT Value)
	{
		CPUSRVescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
		GPUSRVDescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
	}

	void OffsetAll(UINT Value)
	{
		CPUSRVescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
		GPUSRVDescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
		CPURTVDescriptor.Offset(Value, RTVDescriptorSize);
	}
};

class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual void UpdateDescriptors(SRenderObjectDescriptor& OutDescriptor) = 0;
	virtual uint32_t GetNumDescriptors() const = 0;
	virtual void BuildDescriptors(IDescriptor* Descriptor) = 0;
};