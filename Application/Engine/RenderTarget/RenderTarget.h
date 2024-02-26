#pragma once
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"
#include "Engine/UploadBuffer/UploadBuffer.h"

class OCommandQueue;
struct SRenderTargetParams
{
	ID3D12Device* Device;
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
};

class ORenderTargetBase : public ORenderObjectBase
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

	ORenderTargetBase(UINT Width, UINT Height);

	virtual void BuildDescriptors() = 0;
	virtual void BuildResource() = 0;

	ID3D12Resource* GetResource() const;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumRTVRequired() override;
	virtual void InitRenderObject();
	TUUID GetID() override;
	void SetID(TUUID) override;
	void SetViewport(OCommandQueue* CommandQueue) const;

protected:
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	LONG Width = 0;
	LONG Height = 0;
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

	SDescriptorPair GetSRV() const { return SRVHandle; }
	SDescriptorPair GetRTV() const { return RTVHandle; }
	void BuildDescriptors(IDescriptor* Descriptor) override;
	void OnResize(UINT NewWidth, UINT NewHeight);
	void InitRenderObject() override;

protected:
	void BuildDescriptors() override;
	void BuildResource() override;

private:
	SDescriptorPair SRVHandle;
	SDescriptorPair RTVHandle;
};
