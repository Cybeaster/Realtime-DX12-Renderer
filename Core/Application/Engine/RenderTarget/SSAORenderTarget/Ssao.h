#pragma once

#include "Engine/RenderTarget/RenderTarget.h"
class OSSAORenderTarget : public ORenderTargetBase
{
public:
	struct SConstantBufferBlurData
	{
		UINT Horizontal = 0;
		float Pad1;
		float Pad2;
		float Pad3;
	};

	enum ESubtargets
	{
		NormalSubtarget = 0,
		AmbientSubtarget0,
		AmbientSubtarget1
	};

	OSSAORenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
	    : ORenderTargetBase(Device, Width, Height, Format, EResourceHeapType::Default)
	{
		Name = L"SSAORenderTarget";
	}

	void BuildDescriptors(IDescriptor* Descriptor) override;
	void BuildResource() override;
	void BuildDescriptors() override;
	void BuildOffsetVectors();
	void BuildRandomVectorTexture();
	void InitRenderObject() override;
	void BuildViewport() override;
	void OnResize(const ResizeEventArgs& Args) override;

	std::array<DirectX::XMFLOAT4, 14> GetOffsetVectors();
	vector<float> CalcGaussWeights() const;
	static constexpr int MaxBlurRadius = 5;
	SResourceInfo* GetSubresource(uint32_t Idx) override;

	uint32_t GetNumRTVRequired() const override;
	uint32_t GetNumSRVRequired() const override;

	SResourceInfo* GetResource() override;
	SResourceInfo* GetRandomVectorMap();
	SResourceInfo* GetNormalMap();
	SResourceInfo* GetAmbientMap0();
	SResourceInfo* GetDepthMap();

	SDescriptorPair GetNormalMapSRV() const;
	SDescriptorPair GetNormalMapRTV() const;
	SDescriptorPair GetDepthMapSRV() const;
	SDescriptorPair GetAmbientMap0SRV();
	SDescriptorPair GetAmbientMap1SRV();
	SDescriptorPair GetRandomVectorMapSRV() const;
	SDescriptorPair GetAmbientMap0RTV() const;
	SDescriptorPair GetAmbientMap1RTV() const;

	void PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList, uint32_t SubtargetIdx) override;
	D3D12_GPU_VIRTUAL_ADDRESS GetHBlurCBAddress() const;
	D3D12_GPU_VIRTUAL_ADDRESS GetVBlurCBAddress() const;

	float OcclusionRadius = 0.5f;
	float OcclusionFadeStart = 0.2f;
	float OcclusionFadeEnd = 1.0f;
	float SurfaceEpsilon = 0.05f;
	float GauseWeightSigma = 2.5f;

private:
	TUploadBuffer<SConstantBufferBlurData> HBlurCB;
	TUploadBuffer<SConstantBufferBlurData> VBlurCB;

	SResourceInfo RandomVectorMap;
	SResourceInfo RandomVectorMapUploadBuffer;
	SResourceInfo NormalMap;
	SResourceInfo AmbientMap0;
	SResourceInfo AmbientMap1;
	SResourceInfo DepthMap;

	SDescriptorPair NormalMapSRV;
	SDescriptorPair NormalMapRTV;
	SDescriptorPair DepthMapSRV;
	SDescriptorPair RandomVectorMapSRV;

	SDescriptorPair AmbientMap0SRV;
	SDescriptorPair AmbientMap0RTV;
	SDescriptorPair AmbientMap1SRV;
	SDescriptorPair AmbientMap1RTV;

	std::array<DirectX::XMFLOAT4, 14> Offsets{};
};
