#pragma once
#include "Engine/RenderTarget/RenderTarget.h"

class OCubeRenderTarget : public ORenderTargetBase
{
public:
	OCubeRenderTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format, const DirectX::XMUINT2 Res);
	OCubeRenderTarget(const SRenderTargetParams& Params, DirectX::XMUINT2 Res);
	OCubeRenderTarget(const OCubeRenderTarget& rhs) = delete;
	OCubeRenderTarget& operator=(const OCubeRenderTarget& rhs) = delete;

	~OCubeRenderTarget() = default;

	D3D12_VIEWPORT& GetViewport() { return Viewport; }
	D3D12_RECT& GetScissorRect() { return ScissorRect; }

	void BuildDescriptors(IDescriptor* Descriptor) override;

	uint32_t GetNumRTVRequired() override;
	uint32_t GetNumDSVRequired() override;

	void InitRenderObject() override;
	vector<SDescriptorPair>& GetRTVHandle() { return RTVHandle; }
	SDescriptorPair& GetDSVHandle() { return DSVHandle; }
	SDescriptorPair& GetSRVHandle() { return SRVHandle; }
	uint32_t GetNumPassesRequired() override;

protected:
	DirectX::XMUINT2 Resolution;
	void BuildDepthStencilBuffer();

private:
	void BuildViewport();
	void BuildResource() override;
	void BuildDescriptors() override;

	ComPtr<ID3D12Resource> CubeDepthStencilBuffer;

	SDescriptorPair SRVHandle;
	SDescriptorPair DSVHandle;
	vector<SDescriptorPair> RTVHandle;
};
