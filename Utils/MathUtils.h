#pragma once
#include "DXHelper.h"
namespace Utils
{

template<typename T>
T Clamp(const T& x, const T& Low, const T& High)
{
	return x < Low ? Low : (x > High ? High : x);
}

//clang-format off
inline DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
	    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}
//clang-format on

} // namespace Utils