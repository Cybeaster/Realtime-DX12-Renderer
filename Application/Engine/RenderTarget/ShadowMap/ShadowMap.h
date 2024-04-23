#pragma once
#include "Engine/RenderTarget/RenderTarget.h"
class OLightComponent;
class OShadowMap : public ORenderTargetBase
{
public:
	OShadowMap(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format, OLightComponent* InLightComponent);

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
	void SetLightIndex(uint32_t Idx);
	D3D12_GPU_VIRTUAL_ADDRESS GetPassConstantAddresss() const; // TODO propagate to base class
	void SetPassConstant();
	bool ConsumeUpdate();
	void UpdateLightSourceData();
	void SetPassConstants();
private:
	bool bNeedToUpdate = true;
	OLightComponent* LightComponent;
	SPassConstants PassConstant;
	SDescriptorPair SRV;
	SDescriptorPair DSV;
	SResourceInfo RenderTarget;
	TUploadBufferData<SPassConstants> PassConstantBuffer;

	DirectX::XMFLOAT4X4 ShadowTransform;
	float NearZ;
	float FarZ;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Proj;
	DirectX::XMFLOAT3 LightPos;
};
