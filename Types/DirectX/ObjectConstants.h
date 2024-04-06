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

struct SPassConstants
{
	DirectX::XMFLOAT4X4 View = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProjTex = Utils::Math::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerPassPad1; // Use this to pad gEyePosW to 16 bytes

	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
	DirectX::XMFLOAT4 AmbientLight = { 0.4f, 0.4f, 0.6f, 1.0f };

	// Allow application to change fog parameters once per frame.
	// For example, we may only use fog for certain times of day.
	DirectX::XMFLOAT4 FogColor;
	float FogStart;
	float FogRange;

	UINT NumDirLights = 0;
	float cbPerPassPad2; // Padding to align following uints
	UINT NumPointLights = 0;
	UINT NumSpotLights = 0;
	float cbPerPassPad3; // Padding to ensure the cbuffer ends on a 16-byte boundary
	float cbPerPassPad4; // Padding to ensure the cbuffer ends on a 16-byte boundary
};
