#pragma once
#include "DXHelper.h"
#include "Types.h"
class OInputLayout
{
public:
	D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc() const;

private:
	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
};
