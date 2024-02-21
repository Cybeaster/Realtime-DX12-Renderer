#pragma once
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"

struct SRenderTargetParams
{
	ID3D12Device* Device;
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
};

class ORenderTargetBase : public IRenderObject
{
public:
	ORenderTargetBase(ID3D12Device* Device,
	                  UINT Width, UINT Height,
	                  DXGI_FORMAT Format)
	    : Width(Width), Height(Height), Format(Format), Device(Device)
	{
	}

	ORenderTargetBase(const SRenderTargetParams& Params)
	    : Width(Params.Width), Height(Params.Height), Format(Params.Format), Device(Params.Device)
	{
	}

	virtual void BuildDescriptors() = 0;
	virtual void BuildResource() = 0;

	ID3D12Resource* GetResource() const { return RenderTarget.Get(); }

	uint32_t GetNumSRVRequired() const override
	{
		return 1;
	}

	uint32_t GetNumRTVRequired() override
	{
		return 1;
	}
	virtual void Init() = 0;

protected:
	UINT Width = 0;
	UINT Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12Resource> RenderTarget = nullptr;

	ID3D12Device* Device = nullptr;
};

class OOffscreenTexture : public ORenderTargetBase
{
public:
	OOffscreenTexture(ID3D12Device* Device,
	                  UINT Width, UINT Height,
	                  DXGI_FORMAT Format);

	OOffscreenTexture(const OOffscreenTexture& rhs) = delete;
	OOffscreenTexture& operator=(const OOffscreenTexture& rhs) = delete;

	~OOffscreenTexture() = default;

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSRV() const { return GpuSrv; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return CpuRtv; }

	void BuildDescriptors(IDescriptor* Descriptor) override;
	void OnResize(UINT NewWidth, UINT NewHeight);
	void Init() override;

protected:
	void BuildDescriptors() override;
	void BuildResource() override;

private:
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtv;
};
