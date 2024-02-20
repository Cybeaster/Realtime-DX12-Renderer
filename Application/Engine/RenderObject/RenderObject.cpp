#include "RenderObject.h"

CD3DX12_CPU_DESCRIPTOR_HANDLE SRenderObjectDescriptor::OffsetRTV(UINT Value)
{
	const auto old = CPURTVDescriptor;
	CPURTVDescriptor.Offset(Value, RTVDescriptorSize);
	return old;
}

void SRenderObjectDescriptor::OffsetRTV(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPU)
{
	OutCPU = CPURTVDescriptor;
	CPURTVDescriptor.Offset(1, RTVDescriptorSize);
}

void SRenderObjectDescriptor::OffsetSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPU, CD3DX12_GPU_DESCRIPTOR_HANDLE& OutGPU)
{
	OutCPU = CPUSRVescriptor;
	OutGPU = GPUSRVDescriptor;
	CPUSRVescriptor.Offset(1, DSVSRVUAVDescriptorSize);
	GPUSRVDescriptor.Offset(1, DSVSRVUAVDescriptorSize);
}

std::tuple<CD3DX12_CPU_DESCRIPTOR_HANDLE, CD3DX12_GPU_DESCRIPTOR_HANDLE> SRenderObjectDescriptor::OffsetSRV(UINT Value)
{
	auto oldCPU = CPUSRVescriptor;
	auto oldGPU = GPUSRVDescriptor;
	CPUSRVescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
	GPUSRVDescriptor.Offset(Value, DSVSRVUAVDescriptorSize);
	return { oldCPU, oldGPU };
}

void SRenderObjectDescriptor::Init(ID3D12DescriptorHeap* SRVHeap, UINT CBVSRVUAVDescriptorSize, ID3D12DescriptorHeap* RTVHeap, UINT RTVSize)
{
	CPUSRVescriptor = SRVHeap->GetCPUDescriptorHandleForHeapStart();
	GPUSRVDescriptor = SRVHeap->GetGPUDescriptorHandleForHeapStart();
	DSVSRVUAVDescriptorSize = CBVSRVUAVDescriptorSize;

	CPURTVDescriptor = RTVHeap->GetCPUDescriptorHandleForHeapStart();
	RTVDescriptorSize = RTVSize;
}
