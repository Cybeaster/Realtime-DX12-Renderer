#pragma once
#include "Engine/RenderTarget/RenderTarget.h"
class OLightComponent;
class OShadowMap : public ORenderTargetBase
{
public:
	OShadowMap(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format, OLightComponent* InLightComponent)
	    : ORenderTargetBase(Device, Width, Height, Format, EResourceHeapType::Default), LightComponent(InLightComponent)

	{
		Name = L"ShadowMap";
	}

	void BuildDescriptors() override;
	void BuildResource() override;
	void BuildDescriptors(IDescriptor* Descriptor) override;
	SResourceInfo* GetResource() override;
	SDescriptorPair GetSRV(uint32_t SubtargetIdx = 0) const override;
	SDescriptorPair GetDSV(uint32_t SubtargetIdx = 0) const override;

	uint32_t GetNumDSVRequired() const override;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumPassesRequired() const override;
	void PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList, uint32_t SubtargetIdx = 0) override;
	void UpdatePass(const TUploadBufferData<SPassConstants>& Data) override;

	D3D12_GPU_VIRTUAL_ADDRESS GetPassConstantAddresss() const // TODO propagate to base class
	{
		return PassConstantBuffer.Buffer->GetGPUAddress() + PassConstantBuffer.StartIndex * Utils::CalcBufferByteSize(sizeof(SPassConstants));
	}
	void SetPassConstant();

private:
	OLightComponent* LightComponent;
	SPassConstants PassConstant;
	SDescriptorPair SRV;
	SDescriptorPair DSV;
	SResourceInfo RenderTarget;
	TUploadBufferData<SPassConstants> PassConstantBuffer;
};
