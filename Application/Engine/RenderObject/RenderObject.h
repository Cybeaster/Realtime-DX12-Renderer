#pragma once
#include "DXHelper.h"

struct IDescriptor
{
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

	void OffsetAll(UINT Value)
	{
		SRVCPUOffset(Value);
		SRVGPUOffset(Value);
		RTVCPUOffset(Value);
	}
};

class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual uint32_t GetNumDescriptors() const = 0;
	virtual void BuildDescriptors(IDescriptor* Descriptor) = 0;
};