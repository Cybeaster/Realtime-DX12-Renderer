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

	OSSAORenderTarget(const weak_ptr<ODevice>& Device, UINT Width, UINT Height, DXGI_FORMAT Format)
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
	SDescriptorPair GetDSV(uint32_t SubtargetIdx = 0) const override;
	std::array<DirectX::XMFLOAT4, 14> GetOffsetVectors();
	vector<float> CalcGaussWeights() const;
	static constexpr int MaxBlurRadius = 5;
	SResourceInfo* GetSubresource(uint32_t Idx) override;

	uint32_t GetNumRTVRequired() const override;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumDSVRequired() const override;
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

	void PrepareRenderTarget(OCommandQueue* Queue, bool ClearRenderTarget, bool ClearDepth, uint32_t SubtargetIdx) override;
	D3D12_GPU_VIRTUAL_ADDRESS GetHBlurCBAddress() const;
	D3D12_GPU_VIRTUAL_ADDRESS GetVBlurCBAddress() const;

	float OcclusionRadius = 0.5f;
	float OcclusionFadeStart = 0.2f;
	float OcclusionFadeEnd = 1.0f;
	float SurfaceEpsilon = 0.05f;
	float GauseWeightSigma = 2.5f;
	void SetEnabled(bool bEnable);
	bool IsEnabled() const;

private:
	bool Enabled = true;
	TUploadBuffer<SConstantBufferBlurData> HBlurCB;
	TUploadBuffer<SConstantBufferBlurData> VBlurCB;

	TResourceInfo RandomVectorMap;
	TResourceInfo RandomVectorMapUploadBuffer;
	TResourceInfo NormalMap;
	TResourceInfo AmbientMap0;
	TResourceInfo AmbientMap1;
	TResourceInfo DepthMap;

	SDescriptorPair NormalMapSRV;
	SDescriptorPair NormalMapRTV;
	SDescriptorPair DepthMapSRV;
	SDescriptorPair DepthMapDSV;
	SDescriptorPair RandomVectorMapSRV;

	SDescriptorPair AmbientMap0SRV;
	SDescriptorPair AmbientMap0RTV;
	SDescriptorPair AmbientMap1SRV;
	SDescriptorPair AmbientMap1RTV;

	std::array<DirectX::XMFLOAT4, 14> Offsets{};
};
