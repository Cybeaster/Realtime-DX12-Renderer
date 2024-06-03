#pragma once
#include "Engine/RenderTarget/RenderTarget.h"

class OCubeRenderTarget : public ORenderTargetBase
{
public:
	OCubeRenderTarget(const weak_ptr<ODevice>& Device, int Width, int Height, DXGI_FORMAT Format, const DirectX::XMUINT2 Res);
	OCubeRenderTarget(const SRenderTargetParams& Params, DirectX::XMUINT2 Res);
	OCubeRenderTarget(const OCubeRenderTarget& rhs) = delete;
	OCubeRenderTarget& operator=(const OCubeRenderTarget& rhs) = delete;

	~OCubeRenderTarget() = default;

	D3D12_VIEWPORT& GetViewport() { return Viewport; }
	D3D12_RECT& GetScissorRect() { return ScissorRect; }

	void BuildDescriptors(IDescriptor* Descriptor) override;

	uint32_t GetNumRTVRequired() const override;
	uint32_t GetNumDSVRequired() const override;
	uint32_t GetNumSRVRequired() const override;

	SDescriptorPair GetDSV(uint32_t SubtargetIdx = 0) const override;
	SDescriptorPair GetRTV(uint32_t SubtargetIdx = 0) const override;
	SDescriptorPair GetSRV(uint32_t SubtargetIdx = 0) const override;

	void InitRenderObject() override;
	uint32_t GetNumPassesRequired() const override;
	SResourceInfo* GetResource() override { return RenderTarget.get(); }
	virtual void SetBoundRenderItem(const shared_ptr<ORenderItem>& Item);

protected:
	DirectX::XMUINT2 Resolution;
	void BuildDepthStencilBuffer();
	weak_ptr<ORenderItem> RenderItem;

private:
	void BuildResource() override;
	void BuildDescriptors() override;
	TResourceInfo RenderTarget;
	TResourceInfo CubeDepthStencilBuffer;
	SDescriptorPair SRVHandle;
	SDescriptorPair DSVHandle;
	vector<SDescriptorPair> RTVHandle;
};
