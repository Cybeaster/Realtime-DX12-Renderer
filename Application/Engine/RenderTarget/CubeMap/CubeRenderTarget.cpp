
#include "CubeRenderTarget.h"

#include "../../../../Utils/Statics.h"

OCubeRenderTarget::OCubeRenderTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format)
    : ORenderTargetBase(Device, Width, Height, Format)
{
	BuildViewport();
}

OCubeRenderTarget::OCubeRenderTarget(SRenderTargetParams Params)
    : ORenderTargetBase(Params)
{
	BuildViewport();
}

void OCubeRenderTarget::BuildViewport()
{
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = static_cast<float>(Width);
	Viewport.Height = static_cast<float>(Height);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	ScissorRect = { 0, 0, Width, Height };
}

void OCubeRenderTarget::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}
	descriptor->OffsetSRV(HCpuSrv, HGpuSrv);
	descriptor->OffsetRTV(HCpuRtv, GetNumRTVRequired());
	BuildDescriptors();
}

uint32_t OCubeRenderTarget::GetNumRTVRequired()
{
	return 6;
}

uint32_t OCubeRenderTarget::GetNumDSVRequired()
{
	return 1;
}

void OCubeRenderTarget::Init()
{
	BuildResource();
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
	Device->CreateShaderResourceView(RenderTarget.Get(), &srvDesc, HCpuSrv);

	for (int i = 0; i < GetNumRTVRequired(); i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = Format;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;

		// Render target to ith element.
		rtvDesc.Texture2DArray.FirstArraySlice = i;

		// Only view one element of the array.
		rtvDesc.Texture2DArray.ArraySize = 1;

		// Create RTV to ith cubemap face.
		Device->CreateRenderTargetView(RenderTarget.Get(), &rtvDesc, HCpuRtv[i]);
	}
}