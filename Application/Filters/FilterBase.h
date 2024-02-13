#pragma once
#include "CommandQueue/CommandQueue.h"
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"

class OFilterBase : public IRenderObject
{
public:
	virtual ~OFilterBase() = default;

	OFilterBase(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format)
	    : Device(Device), CMDList(List), Width(Width), Height(Height), Format(Format)
	{
	}
	void Init();

	template<typename T>
	static T* CreateFilter(ID3D12Device* _Device, ID3D12GraphicsCommandList* _List, UINT _Width, UINT _Height, DXGI_FORMAT _Format)
	{
		auto newFilter = new T(_Device, _List, _Width, _Height, _Format);
		newFilter->Init();
		return newFilter;
	}

	virtual void OnResize(UINT NewWidth, UINT NewHeight);

protected:
	virtual void BuildDescriptors() const = 0;
	virtual void BuildResource() = 0;

	ID3D12GraphicsCommandList* CMDList = nullptr;
	ID3D12Device* Device = nullptr;

	uint32_t Width = 0;
	uint32_t Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
};

inline void OFilterBase::Init()
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