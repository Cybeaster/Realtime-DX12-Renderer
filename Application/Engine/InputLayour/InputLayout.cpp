
#include "InputLayout.h"
D3D12_INPUT_LAYOUT_DESC OInputLayout::GetInputLayoutDesc() const
{
	D3D12_INPUT_LAYOUT_DESC desc;
	desc.pInputElementDescs = InputLayout.data();
	desc.NumElements = InputLayout.size();
	return desc;
}