#include "NormalTangentDebugTarget.h"
void ONormalTangentDebugTarget::BuildDescriptors(IDescriptor* Descriptor)
{
	auto desc = Cast<SRenderObjectHeap>(Descriptor);
	if (desc == nullptr)
	{
		CWIN_LOG(desc == nullptr, Render, Error, "Descriptor is not of type SDescriptorResourceData!");
		return;
	}
	SRV = desc->SRVHandle.Offset();
	RTV = desc->RTVHandle.Offset();
	BuildDescriptors();
}

void ONormalTangentDebugTarget::BuildResource()
{
	D3D12_RESOURCE_DESC rtvDesc = GetResourceDesc();
	rtvDesc.Format = SRenderConstants::BackBufferFormat;
	rtvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	Target = Utils::CreateResource(this, L"NormalTangentDebugTarget", Device, D3D12_HEAP_TYPE_DEFAULT, rtvDesc, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
SResourceInfo* ONormalTangentDebugTarget::GetResource()
{
	return &Target;
}
SDescriptorPair ONormalTangentDebugTarget::GetSRV(uint32_t SubtargetIdx) const
{
	return SRV;
}
SDescriptorPair ONormalTangentDebugTarget::GetRTV(uint32_t SubtargetIdx) const
{
	return RTV;
}

ONormalTangentDebugTarget::ONormalTangentDebugTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format)
    : ORenderTargetBase(Device, Width, Height, Format, EResourceHeapType::Default)
{
}

uint32_t ONormalTangentDebugTarget::GetNumRTVRequired() const
{
	return 1;
}
uint32_t ONormalTangentDebugTarget::GetNumSRVRequired() const
{
	return 1;
}

void ONormalTangentDebugTarget::InitRenderObject()
{
	BuildResource();
}

void ONormalTangentDebugTarget::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	Device->CreateShaderResourceView(Target.Resource.Get(), &srvDesc, SRV.CPUHandle);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = Format;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;

	Device->CreateRenderTargetView(Target.Resource.Get(), &rtvDesc, RTV.CPUHandle);
}