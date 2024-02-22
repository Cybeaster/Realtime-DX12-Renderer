#pragma once
#include "Filters/FilterBase.h"

class OBilateralBlurFilter : public OFilterBase
{
public:
	OBilateralBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void OutputTo(ID3D12Resource* Destination) const;
	void BuildDescriptors() const override;
	void BuildResource() override;
	void Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO,
	             ID3D12Resource* Input) const;

	uint32_t GetNumSRVRequired() const override
	{
		return 4;
	}

	void SetSpatialSigma(float Value)
	{
		SpatialSigma = Value;
	}
	void SetIntensitySigma(float Value)
	{
		IntensitySigma = Value;
	}
	void SetBlurCount(int32_t Value)
	{
		BlurCount = Value;
	}

private:
	SDescriptorPair BlurOutputSrvHandle;
	SDescriptorPair BlurOutputUavHandle;
	SDescriptorPair BlurInputSrvHandle;
	SDescriptorPair BlurInputUavHandle;

	ComPtr<ID3D12Resource> InputTexture;
	ComPtr<ID3D12Resource> OutputTexture;

	float SpatialSigma;
	float IntensitySigma;
	int32_t BlurCount;
};
