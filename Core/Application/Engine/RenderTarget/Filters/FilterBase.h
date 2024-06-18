#pragma once
#include "CommandQueue/CommandQueue.h"
#include "DirectX/DXHelper.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
class ODevice;
class OFilterBase : public ORenderObjectBase
{
public:
	virtual ~OFilterBase() = default;

	OFilterBase(const weak_ptr<ODevice>& Device, const shared_ptr<OCommandQueue>& Other, UINT Width, UINT Height, DXGI_FORMAT Format)
	    : Device(Device), Queue(Other), Width(Width), Height(Height), Format(Format)
	{
	}
	void InitRenderObject();

	template<typename T>
	static shared_ptr<T> CreateFilter(const shared_ptr<ODevice>& InDevice, shared_ptr<OCommandQueue> InQueue, UINT InWidth, UINT InHeight, DXGI_FORMAT InFormat)
	{
		auto newFilter = make_shared<T>(InDevice, InQueue, InWidth, InHeight, InFormat);
		newFilter->InitRenderObject();
		return newFilter;
	}

	virtual void OnResize(UINT NewWidth, UINT NewHeight);
	wstring GetName() const override
	{
		return FilterName;
	}

protected:
	virtual void BuildDescriptors() = 0;
	virtual void BuildResource() = 0;

	weak_ptr<OCommandQueue> Queue;
	weak_ptr<ODevice> Device;
	uint32_t Width = 0;
	uint32_t Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	wstring FilterName;
};

inline void OFilterBase::InitRenderObject()
{
	BuildResource();
}

inline void OFilterBase::OnResize(UINT NewWidth, UINT NewHeight)
{
	if (NewWidth != Width || NewHeight != Height)
	{
		Width = NewWidth;
		Height = NewHeight;

		BuildResource();
		BuildDescriptors();
	}
}