#pragma once
#include "Light/Light.h"
#include "MathUtils.h"
#include "RenderConstants.h"

#include <DirectXMath.h>

struct SObjectConstants
{
	DirectX::XMFLOAT4X4 World = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Utils::Math::Identity4x4();
	UINT MaterialIndex;
	UINT ObjPad0;
	UINT ObjPad1;
	UINT ObjPad2;
};
