#pragma once
#include "Engine/RenderTarget/RenderTarget.h"

class OCubeRenderTarget : public ORenderTargetBase
{
public:
	OCubeRenderTarget(ID3D12Device* Device, int Width, int Height, DXGI_FORMAT Format);
	OCubeRenderTarget(SRenderTargetParams Params);
	OCubeRenderTarget(const OCubeRenderTarget& rhs) = delete;
	OCubeRenderTarget& operator=(const OCubeRenderTarget& rhs) = delete;

	~OCubeRenderTarget() = default;

	D3D12_VIEWPORT GetViewport() const { return Viewport; }
	D3D12_RECT GetScissorRect() const { return ScissorRect; }

	void BuildDescriptors(IDescriptor* Descriptor) override;

	uint32_t GetNumRTVRequired() override;
	uint32_t GetNumDSVRequired() override;

	void Init() override;

private:
	void BuildViewport();
	void BuildResource() override;
	void BuildDescriptors() override;

	ID3D12Device* Device = nullptr;
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	CD3DX12_CPU_DESCRIPTOR_HANDLE HCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE HGpuSrv;
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> HCpuRtv{ GetNumRTVRequired() };
};
