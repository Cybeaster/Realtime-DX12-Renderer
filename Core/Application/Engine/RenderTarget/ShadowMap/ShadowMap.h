#pragma once
#include "Engine/RenderTarget/RenderTarget.h"
class OLightComponent;
class OShadowMap : public ORenderTargetBase
{
public:
	OShadowMap(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format);

	void BuildDescriptors() override;
	void BuildResource() override;
	void BuildDescriptors(IDescriptor* Descriptor) override;
	SResourceInfo* GetResource() override;
	SDescriptorPair GetSRV(uint32_t SubtargetIdx = 0) const override;
	SDescriptorPair GetDSV(uint32_t SubtargetIdx = 0) const override;

	uint32_t GetNumDSVRequired() const override;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumPassesRequired() const override;
	D3D12_GPU_VIRTUAL_ADDRESS GetPassConstantAddresss() const; // TODO propagate to base class
	void PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList, uint32_t SubtargetIdx = 0) override;
	void UpdatePass(const TUploadBufferData<SPassConstants>& Data) override;
	void SetShadowMapIndex(uint32_t Idx);

	bool ConsumeUpdate();
	void UpdateLightSourceData();
	void SetPassConstants(const SPassConstants&);
	uint32_t GetShadowMapIndex() const;
	bool IsValid();

private:
	bool bNeedToUpdate = true;
	SPassConstants PassConstant;
	SDescriptorPair SRV;
	SDescriptorPair DSV;
	SResourceInfo RenderTarget;
	TUploadBufferData<SPassConstants> PassConstantBuffer;
	std::optional<uint32_t> ShadowMapIndex;
	float NearZ;
	float FarZ;
	DirectX::XMFLOAT4X4 ShadowTransform;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Proj;
};
