#pragma once
#include "DXHelper.h"
#include "Filters/FilterBase.h"
class OSobelFilter : public OFilterBase
{
public:
	OSobelFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);
	OSobelFilter(const OSobelFilter& rhs) = delete;
	OSobelFilter& operator=(const OSobelFilter& rhs) = delete;

	CD3DX12_GPU_DESCRIPTOR_HANDLE OutputSRV() const;

	uint32_t GetNumSRVRequired() const override
	{
		return 2;
	}

	void BuildDescriptors() const override;
	void BuildResource() override;

	std::pair<bool, CD3DX12_GPU_DESCRIPTOR_HANDLE> Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, CD3DX12_GPU_DESCRIPTOR_HANDLE Input) const;
	void BuildDescriptors(IDescriptor* Descriptor) override;

	void SetIsEnabled(bool bIsEnabled)
	{
		bEnabled = bIsEnabled;
	}

private:
	SDescriptorPair SRVHandle;
	SDescriptorPair UAVHandle;

	ComPtr<ID3D12Resource> Output = nullptr;
	bool bEnabled = true;
};
