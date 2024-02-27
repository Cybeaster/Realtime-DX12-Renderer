//
// Created by Cybea on 27/02/2024.
//

#include "InputLayout.h"
D3D12_INPUT_LAYOUT_DESC OInputLayout::GetInputLayoutDesc() const
{
	D3D12_INPUT_LAYOUT_DESC desc;
	desc.pInputElementDescs = InputLayout.data();
	desc.NumElements = InputLayout.size();
	return desc;
}