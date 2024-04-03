#pragma once
#include "DXHelper.h"

class IRenderObject;
struct SResourceInfo
{
	void Init(IRenderObject* InContext, D3D12_RESOURCE_STATES InState)
	{
		Context = InContext;
		CurrentState = InState;
	}

	ComPtr<ID3D12Resource> operator=(ComPtr<ID3D12Resource> InResource)
	{
		Resource = InResource;
		return Resource;
	}

	D3D12_RESOURCE_STATES CurrentState;
	ComPtr<ID3D12Resource> Resource;
	IRenderObject* Context;
};
