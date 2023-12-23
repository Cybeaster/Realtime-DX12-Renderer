#pragma once
#include "../Utils/MathUtils.h"

#include <DirectXMath.h>

struct SObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = Utils::Identity4x4();
};

struct SPassConstants
{
};

struct STimerConstants
{
	float Time = 0.0f;
};