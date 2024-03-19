#pragma once
#include <DirectXMath.h>



struct SSpotLight
{
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	float pad1=0; // Padding to align with HLSL's float4
	DirectX::XMFLOAT3 Direction =  { 0.0f, -1.0f, 0.0f };
	float pad2=0; // Padding to align with HLSL's float4
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FallOffStart =  1.0f;
	float FallOffEnd =  10.0f;
	float SpotPower =  64.0f;
	float pad3 =0; // Padding to make the structure size a multiple of 16 bytes
};

struct SPointLight
{
	DirectX::XMFLOAT3 Position= { 0.0f, 0.0f, 0.0f };
	float pad1=0; // Padding to align with HLSL's float4
	DirectX::XMFLOAT3 Strength=  { 0.5f, 0.5f, 0.5f };
	float FallOffStart= 1.0f;
	float FallOffEnd= 10.0f;
};

struct SDirectionalLight
{
	DirectX::XMFLOAT3 Direction= { 0.57735f, -0.57735f, 0.57735f };
	float pad1=0;// Padding to align with HLSL's float4
	DirectX::XMFLOAT3 Strength= { 0.5f, 0.5f, 0.5f };
	float pad2=0;// Padding to make the structure size a multiple of 16 bytes
};




