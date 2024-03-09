#pragma once
#include "Filters/FilterBase.h"

class OBilateralBlurFilter : public OFilterBase
{
public:
	OBilateralBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void OutputTo(SResourceInfo* Destination) const;
	void BuildDescriptors() const override;
	void BuildResource() override;
	void Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO,
	             SResourceInfo*);
	void Execute(SResourceInfo*);
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
	string GetName() override
	{
		return "BilateralBlur";
	}

private:
	SDescriptorPair BlurOutputSrvHandle;
	SDescriptorPair BlurOutputUavHandle;
	SDescriptorPair BlurInputSrvHandle;
	SDescriptorPair BlurInputUavHandle;

	SResourceInfo InputTexture;
	SResourceInfo OutputTexture;

	float SpatialSigma;
	float IntensitySigma;
	int32_t BlurCount;
};
