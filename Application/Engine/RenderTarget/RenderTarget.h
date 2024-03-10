#pragma once
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

	virtual SResourceInfo* GetResource() = 0;
	uint32_t GetNumSRVRequired() const override;
	uint32_t GetNumRTVRequired() override;
	uint32_t GetNumDSVRequired() override;
	void CopyTo(ORenderTargetBase* Dest, const OCommandQueue* CommandQueue);
	virtual SDescriptorPair GetSRV() const;
	virtual SDescriptorPair GetRTV() const;
	virtual SDescriptorPair GetDSV() const;
	virtual void InitRenderObject();

	TUUID GetID() override;
	void SetID(TUUID) override;
	void SetViewport(ID3D12GraphicsCommandList* List) const;
	void PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList);
	void UnsetRenderTarget(OCommandQueue* CommandQueue);

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
	string GetName() override
	{
		return "OffscreenTexture";
	}

protected:
	void BuildDescriptors() override;
	void BuildResource() override;

public:
	SResourceInfo* GetResource() override;

private:
	SDescriptorPair SRVHandle;
	SDescriptorPair RTVHandle;
	SDescriptorPair DSVHandle;

	// Two for ping-ponging the textures.
	SResourceInfo RenderTarget;
};
