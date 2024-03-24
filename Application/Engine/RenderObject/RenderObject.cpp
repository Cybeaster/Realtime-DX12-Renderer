#include "RenderObject.h"

void SRenderObjectHeap::Init(UINT SRVCBVUAVDescSize, UINT RTVDescSize, UINT DSVDescSize)
{
	if (SRVHeap)
	{
		SRVHandle.Init({ SRVHeap.Get(), SRVCBVUAVDescSize, SRVHeap->GetDesc().NumDescriptors });
	}
	if (RTVHeap)
	{
		RTVHandle.Init({ RTVHeap.Get(), RTVDescSize, RTVHeap->GetDesc().NumDescriptors });
	}
	if (DSVHeap)
	{
		DSVHandle.Init({ DSVHeap.Get(), DSVDescSize, DSVHeap->GetDesc().NumDescriptors });
	}
}
