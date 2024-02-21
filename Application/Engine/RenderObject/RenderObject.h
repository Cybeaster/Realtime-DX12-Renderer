#pragma once
#include "../Types/DirectX/DXHelper.h"

struct IDescriptor
{
	virtual ~IDescriptor() = default;
};

struct SRenderObjectDescriptor : IDescriptor
{
	void Init(ID3D12DescriptorHeap* SRVHeap, UINT CBVSRVUAVDescriptorSize, ID3D12DescriptorHeap* RTVHeap, UINT RTVSize);

	CD3DX12_CPU_DESCRIPTOR_HANDLE OffsetRTV(UINT Value = 1);
	void OffsetRTV(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPU);
	void OffsetRTV(vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>& OutCPU);
	void OffsetRTV(vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>& OutCPU, uint32_t NumDesc);

	void OffsetSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPU, CD3DX12_GPU_DESCRIPTOR_HANDLE& OutGPU);
	std::tuple<CD3DX12_CPU_DESCRIPTOR_HANDLE, CD3DX12_GPU_DESCRIPTOR_HANDLE> OffsetSRV(UINT Value = 1);

private:
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUSRVescriptor;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUSRVDescriptor;

	CD3DX12_CPU_DESCRIPTOR_HANDLE CPURTVDescriptor;

	UINT DSVSRVUAVDescriptorSize = 0;
	UINT RTVDescriptorSize = 0;
};

class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual void BuildDescriptors(IDescriptor* Descriptor){};
	virtual uint32_t GetNumSRVRequired() const = 0;
	virtual uint32_t GetNumRTVRequired() { return 0; }
	virtual uint32_t GetNumDSVRequired() { return 0; }
};