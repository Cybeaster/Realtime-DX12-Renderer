#pragma once
#include "../../Utils/Math.h"

#include <DirectXMath.h>

struct SInstanceData
{
	DirectX::XMFLOAT4X4 World = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Utils::Math::Identity4x4();
	UINT MaterialIndex;
	UINT Pad0;
	UINT Pad1;
	UINT Pad2;
};