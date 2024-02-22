#include "RenderObject.h"

void SRenderObjectDescriptor::Init(SDescriptorResourceData SRV, SDescriptorResourceData RTV, SDescriptorResourceData DSV)
{
	SRVHandle.Init(SRV);
	RTVHandle.Init(RTV);
	DSVHandle.Init(DSV);
}
