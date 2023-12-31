#pragma once
#include <DirectXMath.h>


struct SLight
{
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f }; //Light Color
	float FallOffStart = 1.0f; // point/spot light only

	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f }; // directional/spot light only
	float FallOffEnd = 10.0f; // point/spot light only

	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f }; // point/spot light only
	float SpotPower = 64.0f; // spot light only
};
