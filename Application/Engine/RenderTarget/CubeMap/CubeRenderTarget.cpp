
#include "CubeRenderTarget.h"

#include "../../../../Utils/Statics.h"

void OCubeRenderTarget::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}
	descriptor->OffsetSRV(HCpuSrv, HGpuSrv);
}

uint32_t OCubeRenderTarget::GetNumRTVRequired()
{
	return 6;
}

uint32_t OCubeRenderTarget::GetNumDSVRequired()
{
	return 1;
}

void OCubeRenderTarget::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.DepthOrArraySize = GetNumRTVRequired();
	texDesc.MipLevels = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	const auto defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(Device->CreateCommittedResource(&defaultHeapProp,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                nullptr,
	                                                IID_PPV_ARGS(RenderTarget.GetAddressOf())));
}

void OCubeRenderTarget::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
}