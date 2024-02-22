#pragma once
#include "Filters/FilterBase.h"

#include <Types.h>
#include <d3dx12.h>
#include <wrl/client.h>
class OBlurFilter : public OFilterBase
{
public:
	OBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);
	OBlurFilter(const OBlurFilter& rhs) = delete;
	OBlurFilter& operator=(const OBlurFilter& rhs) = delete;

	void OutputTo(ID3D12Resource* Destination) const;

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void Execute(
	    ID3D12RootSignature* RootSignature,
	    ID3D12PipelineState* HorizontalBlurPSO,
	    ID3D12PipelineState* VerticalBlurPSO,
	    ID3D12Resource* Input) const;
	uint32_t GetNumSRVRequired() const override
	{
		return 4;
	}

	void SetParameters(float InSigma, uint32_t InBlurCount)
	{
		Sigma = InSigma;
		BlurCount = InBlurCount;
	}

private:
	vector<float> CalcGaussWeights(float Sigma) const;

	void BuildDescriptors() const override;
	void BuildResource() override;

private:
	const uint32_t MaxBlurRadius = 5;

	SDescriptorPair SRV0Handle;
	SDescriptorPair UAV0Handle;

	SDescriptorPair SRV1Handle;
	SDescriptorPair UAV1Handle;

	Microsoft::WRL::ComPtr<ID3D12Resource> BlurMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> BlurMap1 = nullptr;

	float Sigma = 2.5;
	uint32_t BlurCount = 1;
};
