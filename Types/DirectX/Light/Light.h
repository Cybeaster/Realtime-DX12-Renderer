#pragma once
#include <DirectXMath.h>

struct SSpotLightPayload
{
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FallOffStart = 1.0f;
	float FallOffEnd = 10.0f;
	float SpotPower = 64.0f;
	float ConeAngle = 45.f;
};

struct SPointLightPayload
{
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Intensity = { 0.5f, 0.5f, 0.5f };
	float FallOffStart = 1.0f;
	float FallOffEnd = 10.0f;
};

struct SDirectionalLightPayload
{
	DirectX::XMFLOAT3 Direction = { 0.57735f, -0.57735f, 0.57735f };
	DirectX::XMFLOAT3 Intensity = { 0.5f, 0.5f, 0.5f };
};