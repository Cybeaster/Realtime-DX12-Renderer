
#include "ShadowMap.h"

#include "Engine/Engine.h"
#include "LightComponent/LightComponent.h"
void OShadowMap::BuildResource()
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
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	RenderTarget = Utils::CreateResource(this, L"ShadowMap_RenderTarget", Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear);
}

OShadowMap::OShadowMap(ID3D12Device* Device, UINT ShadowMapSize, DXGI_FORMAT Format)
    : ORenderTargetBase(Device, ShadowMapSize, ShadowMapSize, Format, EResourceHeapType::Default), MapSize(ShadowMapSize)
{
	Name = L"ShadowMap";
	ShadowMapInstancesBuffer = OUploadBuffer<HLSL::InstanceData>::Create(Device, OEngine::Get()->GetAllRenderItems().size(), false, this, L"_ShadowMapInstancesBuffer");
}

void OShadowMap::BuildDescriptors()
{
	// Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	Device->CreateShaderResourceView(RenderTarget.Resource.Get(), &srvDesc, SRV.CPUHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(RenderTarget.Resource.Get(), &dsvDesc, DSV.CPUHandle);
}

void OShadowMap::BuildDescriptors(IDescriptor* Descriptor)
{
	auto desc = Cast<SRenderObjectHeap>(Descriptor);
	if (desc)
	{
		SRV = desc->SRVHandle.Offset();
		DSV = desc->DSVHandle.Offset();
		BuildResource();
		BuildDescriptors();
	}
}

SDescriptorPair OShadowMap::GetSRV(uint32_t SubtargetIdx) const
{
	return SRV;
}

SDescriptorPair OShadowMap::GetDSV(uint32_t SubtargetIdx) const
{
	return DSV;
}

SResourceInfo* OShadowMap::GetResource()
{
	return &RenderTarget;
}

void OShadowMap::PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList, uint32_t SubtargetIdx)
{
	if (PreparedTaregts.contains(SubtargetIdx))
	{
		return;
	}
	PreparedTaregts.insert(SubtargetIdx);
	auto depthStencilView = GetDSV(SubtargetIdx);
	Utils::ResourceBarrier(CommandList, GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	CommandList->ClearDepthStencilView(depthStencilView.CPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	CommandList->OMSetRenderTargets(0, nullptr, true, &depthStencilView.CPUHandle);
}

void OShadowMap::UpdatePass(const TUploadBufferData<SPassConstants>& Data)
{
	Data.Buffer->CopyData(Data.StartIndex, PassConstant);
	PassConstantBuffer = Data;
}

bool OShadowMap::ConsumeUpdate()
{
	if (bNeedToUpdate)
	{
		bNeedToUpdate = false;
		return true;
	}
	return false;
}

void OShadowMap::UpdateLightSourceData()
{
}

void OShadowMap::SetPassConstants(const SPassConstants& Pass)
{
	PassConstant = Pass;
	PassConstant.RenderTargetSize = DirectX::XMFLOAT2(static_cast<float>(Width), static_cast<float>(Height));
	PassConstant.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / Width, 1.0f / Height);
	InstancesInfo = OEngine::Get()->PerformFrustumCulling(BoundingGeometry.get(), LightView, ShadowMapInstancesBuffer);
	bNeedToUpdate = true;
}

uint32_t OShadowMap::GetShadowMapIndex() const
{
	return ShadowMapIndex.value_or(-1);
}

bool OShadowMap::IsValid()
{
	return ShadowMapIndex.has_value();
}

UINT OShadowMap::GetMapSize() const
{
	return MapSize;
}

void OShadowMap::UpdateBoundingGeometry(IBoundingGeometry* InGeo, const DirectX::XMMATRIX& InLightView)
{
	BoundingGeometry = InGeo->Clone();
	LightView = InLightView;
}

IBoundingGeometry* OShadowMap::GetBoundingGeometry() const
{
	return BoundingGeometry.get();
}

SCulledInstancesInfo& OShadowMap::GetCulledInstancesInfo()
{
	return InstancesInfo;
}

uint32_t OShadowMap::GetNumDSVRequired() const
{
	return 1;
}

uint32_t OShadowMap::GetNumSRVRequired() const
{
	return 1;
}

uint32_t OShadowMap::GetNumPassesRequired() const
{
	return 1;
}

void OShadowMap::SetShadowMapIndex(uint32_t Idx)
{
	ShadowMapIndex = Idx;
}

D3D12_GPU_VIRTUAL_ADDRESS OShadowMap::GetPassConstantAddresss() const
{
	return PassConstantBuffer.Buffer->GetGPUAddress() + PassConstantBuffer.StartIndex * Utils::CalcBufferByteSize(sizeof(SPassConstants));
}
