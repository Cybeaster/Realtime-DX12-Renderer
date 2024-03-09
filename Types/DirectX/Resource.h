#pragma once
#include "DXHelper.h"

class IRenderObject;
struct SResourceInfo
{
	D3D12_RESOURCE_STATES CurrentState;
	ComPtr<ID3D12Resource> Resource;
	IRenderObject* Context;
};
