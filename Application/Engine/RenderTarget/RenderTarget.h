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

	virtual ID3D12Resource* GetResource() const = 0;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumRTVRequired() override;
	uint32_t GetNumDSVRequired() override;
	void CopyTo(const ORenderTargetBase* Dest, const OCommandQueue* CommandQueue);
	virtual SDescriptorPair GetSRV() const;
	virtual SDescriptorPair GetRTV() const;
	virtual SDescriptorPair GetDSV() const;
	virtual void InitRenderObject();

	TUUID GetID() override;
	void SetID(TUUID) override;
	void SetViewport(OCommandQueue* CommandQueue) const;
	void PrepareRenderTarget(OCommandQueue* CommandQueue);

protected:
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	LONG Width = 0;
	LONG Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D12Device* Device = nullptr;
	bool HasBeedPrepared = false;
};

class OOffscreenTexture : public ORenderTargetBase
{
public:
	OOffscreenTexture(ID3D12Device* Device,
	                  UINT Width, UINT Height,
	                  DXGI_FORMAT Format);

	OOffscreenTexture(const OOffscreenTexture& rhs) = delete;
	OOffscreenTexture& operator=(const OOffscreenTexture& rhs) = delete;

	~OOffscreenTexture() override;

	SDescriptorPair GetSRV() const { return SRVHandle; }
	SDescriptorPair GetRTV() const { return RTVHandle; }
	SDescriptorPair GetDSV() const { return DSVHandle; }
	void BuildDescriptors(IDescriptor* Descriptor) override;
	void OnResize(UINT NewWidth, UINT NewHeight);
	void InitRenderObject() override;

protected:
	void BuildDescriptors() override;
	void BuildResource() override;

public:
	ID3D12Resource* GetResource() const override;

private:
	SDescriptorPair SRVHandle;
	SDescriptorPair RTVHandle;
	SDescriptorPair DSVHandle;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12Resource> RenderTarget = nullptr;
};
