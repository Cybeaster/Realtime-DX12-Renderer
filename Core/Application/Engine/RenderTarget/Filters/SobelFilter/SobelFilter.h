#pragma once
#include "Engine/RenderTarget/Filters/FilterBase.h"
class OSobelFilter : public OFilterBase
{
public:
	OSobelFilter(const weak_ptr<ODevice>& Device, const shared_ptr<OCommandQueue>& Other, UINT Width, UINT Height, DXGI_FORMAT Format);
	OSobelFilter(const OSobelFilter& rhs) = delete;
	OSobelFilter& operator=(const OSobelFilter& rhs) = delete;

	CD3DX12_GPU_DESCRIPTOR_HANDLE OutputSRV() const;

	uint32_t GetNumSRVRequired() const override
	{
		return 3;
	}

	void BuildDescriptors() override;
	void BuildResource() override;

	bool Execute(SPSODescriptionBase* PSO, ORenderTargetBase* InTarget);

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void SetIsEnabled(bool bIsEnabled, bool InPureSobel)
	{
		bEnabled = bIsEnabled;
		PureSobel = InPureSobel;
	}

	bool IsEnabled() const;
	bool IsPureSobel() const;

	auto GetOutputSRV() const
	{
		return OutputSRVHandle;
	}

	auto GetInputSRV() const
	{
		return InputSRVHandle;
	}

	SResourceInfo* GetOutput() const;

private:
	SDescriptorPair OutputSRVHandle;
	SDescriptorPair OutputUAVHandle;
	SDescriptorPair InputSRVHandle;

	TResourceInfo Output;
	TResourceInfo Input;
	bool bEnabled = true;
	bool PureSobel = false;
};
