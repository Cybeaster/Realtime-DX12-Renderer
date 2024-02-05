#pragma once
#include "..\..\Utils\Math.h"
#include "Light/Light.h"
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

struct SPassConstants
{
	DirectX::XMFLOAT4X4 View = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = Utils::Math::Identity4x4();

	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float CBPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Allow application to change fog parameters once per frame.
	// For example, we may only use fog for certain times of day.
	DirectX::XMFLOAT4 FogColor;
	float FogStart;
	float FogRange;

	DirectX::XMFLOAT2 cbPerPassPad2;
	SLight Lights[SRenderConstants::MaxLights];
};
