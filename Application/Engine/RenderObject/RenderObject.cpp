#include "RenderObject.h"

void SRenderObjectDescriptor::Init(ID3D12DescriptorHeap* SRVHeap, ID3D12DescriptorHeap* DSVHeap, ID3D12DescriptorHeap* RTVHeap, UINT SRVSize, UINT RTVSize, UINT DSVSize)
{
	SRVHandle.DescSize = SRVSize;
	SRVHandle.CPUHandle = SRVHeap->GetCPUDescriptorHandleForHeapStart();
	SRVHandle.GPUHandle = SRVHeap->GetGPUDescriptorHandleForHeapStart();

	RTVHandle.DescSize = RTVSize;
	RTVHandle.CPUHandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();
	RTVHandle.GPUHandle = RTVHeap->GetGPUDescriptorHandleForHeapStart();

	DSVHandle.DescSize = DSVSize;
	DSVHandle.CPUHandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
	DSVHandle.GPUHandle = DSVHeap->GetGPUDescriptorHandleForHeapStart();
}
