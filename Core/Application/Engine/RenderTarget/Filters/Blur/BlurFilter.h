#pragma once
#include "Engine/RenderTarget/Filters/FilterBase.h"
#include "Types.h"
#include "d3dx12.h"

#include <wrl/client.h>

class ODevice;
struct SConstantBlurSettings
{
	int BlurRadius;
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

class OGaussianBlurFilter : public OFilterBase
{
public:
	OGaussianBlurFilter(const shared_ptr<ODevice>& Device, const shared_ptr<OCommandQueue>& Other, UINT Width, UINT Height, DXGI_FORMAT Format);
	OGaussianBlurFilter(const OGaussianBlurFilter& rhs) = delete;
	OGaussianBlurFilter& operator=(const OGaussianBlurFilter& rhs) = delete;
	void InitRenderObject() override;
	void OutputTo(SResourceInfo* Destination);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	bool Execute(
	    const SPSODescriptionBase* HorizontalBlurPSO,
	    const SPSODescriptionBase* VerticalBlurPSO,
	    SResourceInfo* Input);
	uint32_t GetNumSRVRequired() const override
	{
		return 5;
	}

	void SetParameters(float InSigma, uint32_t InBlurCount)
	{
		Sigma = InSigma;
		BlurCount = InBlurCount;
	}

private:
	vector<float> CalcGaussWeights(float Sigma) const;

	void BuildDescriptors() override;
	void BuildResource() override;

private:
	const uint32_t MaxBlurRadius = 5;

	SDescriptorPair SRV0Handle;
	SDescriptorPair UAV0Handle;

	SDescriptorPair SRV1Handle;
	SDescriptorPair UAV1Handle;
	SDescriptorPair InputSRVHandle;

	TResourceInfo BlurMap0;
	TResourceInfo BlurMap1;

	TResourceInfo InputMap;

	TUploadBuffer<SConstantBlurSettings> Buffer;

	float Sigma = 2.5;
	uint32_t BlurCount = 1;
};
