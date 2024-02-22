#pragma once
#include "Engine/RenderTarget/RenderTarget.h"

class OCubeRenderTarget : public ORenderTargetBase
{
public:
	OCubeRenderTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format);
	OCubeRenderTarget(const SRenderTargetParams& Params);
	OCubeRenderTarget(const OCubeRenderTarget& rhs) = delete;
	OCubeRenderTarget& operator=(const OCubeRenderTarget& rhs) = delete;

	~OCubeRenderTarget() = default;

	D3D12_VIEWPORT& GetViewport() { return Viewport; }
	D3D12_RECT& GetScissorRect() { return ScissorRect; }

	void BuildDescriptors(IDescriptor* Descriptor) override;

	uint32_t GetNumRTVRequired() override;
	uint32_t GetNumDSVRequired() override;

	void Init() override;
	vector<SDescriptorPair>& GetRTVHandle() { return RTVHandle; }
	SDescriptorPair& GetDSVHandle() { return DSVHandle; }
	SDescriptorPair& GetSRVHandle() { return SRVHandle; }

private:
	void BuildViewport();
	void BuildResource() override;
	void BuildDescriptors() override;

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	SDescriptorPair SRVHandle;
	SDescriptorPair DSVHandle;
	vector<SDescriptorPair> RTVHandle;
};
