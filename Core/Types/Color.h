#pragma once
#include "DirectX/DXHelper.h"

struct SColor
{
	SColor() = default;
	SColor(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255)
	    : R(R), G(G), B(B), A(A) {}

	static const SColor White, Black, Red, Green, Blue;

	DirectX::XMFLOAT3 ToFloat3() const
	{
		return DirectX::XMFLOAT3(R / 255.0f, G / 255.0f, B / 255.0f);
	}

	DirectX::XMFLOAT4 ToFloat4() const
	{
		return DirectX::XMFLOAT4(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
	}

	uint8_t R = 0;
	uint8_t G = 0;
	uint8_t B = 0;
	uint8_t A = 255;
};

inline const SColor SColor::White = SColor(255, 255, 255);
inline const SColor SColor::Black = SColor(0, 0, 0);
inline const SColor SColor::Red = SColor(255, 0, 0);
inline const SColor SColor::Green = SColor(0, 255, 0);
inline const SColor SColor::Blue = SColor(0, 0, 255);
