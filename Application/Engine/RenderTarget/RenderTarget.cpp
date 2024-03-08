//
// Created by Cybea on 24/01/2024.
//

#include "RenderTarget.h"

#include "../../../Utils/Statics.h"
#include "Engine/Engine.h"

ORenderTargetBase::ORenderTargetBase(UINT Width, UINT Height)
    : Width(Width), Height(Height), Format(SRenderConstants::BackBufferFormat), Device(OEngine::Get()->GetDevice().Get())
{
}

uint32_t ORenderTargetBase::GetNumSRVRequired() const
{
	return 1;
}

uint32_t ORenderTargetBase::GetNumRTVRequired()
{
	return 1;
}
uint32_t ORenderTargetBase::GetNumDSVRequired()
{
	return 1;
}
void ORenderTargetBase::CopyTo(const ORenderTargetBase* Dest, const OCommandQueue* CommandQueue)
{
	CommandQueue->CopyResourceTo(Dest, this);
}
SDescriptorPair ORenderTargetBase::GetSRV() const
{
	LOG(Render, Error, "GetSRV not implemented")
	return {};
}
SDescriptorPair ORenderTargetBase::GetRTV() const
{
	LOG(Render, Error, "GetRTV not implemented")
	return {};
}

SDescriptorPair ORenderTargetBase::GetDSV() const
{
	LOG(Render, Error, "GetDSV not implemented")
	return {};
}

void ORenderTargetBase::InitRenderObject()
{
}

void ORenderTargetBase::SetViewport(OCommandQueue* CommandQueue) const
{
	auto list = CommandQueue->GetCommandList();
	list->RSSetViewports(1, &Viewport);
	list->RSSetScissorRects(1, &ScissorRect);
}

void ORenderTargetBase::PrepareRenderTarget(OCommandQueue* CommandQueue)
{
	if (HasBeedPrepared)
	{
		return;
	}

	auto cmdList = CommandQueue->GetCommandList();
	auto backbufferView = GetRTV().CPUHandle;
	auto dsv = GetDSV().CPUHandle;
	cmdList->ClearRenderTargetView(backbufferView, DirectX::Colors::LightSteelBlue, 0, nullptr);
	cmdList->ClearDepthStencilView(GetDSV().CPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &backbufferView, true, &dsv);
	Utils::ResourceBarrier(cmdList.Get(), GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	HasBeedPrepared = true;
}

TUUID ORenderTargetBase::GetID()
{
	return ID;
}

void ORenderTargetBase::SetID(TUUID Other)
{
	ID = Other;
}

OOffscreenTexture::OOffscreenTexture(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
    : ORenderTargetBase(Device, Width, Height, Format)
{
}
OOffscreenTexture::~OOffscreenTexture()
{
}

void OOffscreenTexture::BuildDescriptors(IDescriptor* Descriptor)
{
	if (const auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor))
	{
		descriptor->SRVHandle.Offset(SRVHandle);
		descriptor->RTVHandle.Offset(RTVHandle);
		descriptor->DSVHandle.Offset(DSVHandle);
		BuildDescriptors();
	}
}

void OOffscreenTexture::OnResize(UINT NewWidth, UINT NewHeight)
{
	if (Width != NewWidth || Height != NewHeight)
	{
		Width = NewWidth;
		Height = NewHeight;

		BuildResource();
		BuildDescriptors();
	}
}

void OOffscreenTexture::InitRenderObject()
{
	ORenderTargetBase::InitRenderObject();
	BuildResource();
}

void OOffscreenTexture::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	Device->CreateShaderResourceView(RenderTarget.Get(), &srvDesc, SRVHandle.CPUHandle);
	Device->CreateRenderTargetView(RenderTarget.Get(), nullptr, RTVHandle.CPUHandle);
}

void OOffscreenTexture::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(Device->CreateCommittedResource(&defaultHeap,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                nullptr,
	                                                IID_PPV_ARGS(&RenderTarget)));
}
ID3D12Resource* OOffscreenTexture::GetResource() const
{
	return RenderTarget.Get();
}
