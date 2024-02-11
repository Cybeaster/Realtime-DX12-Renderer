#pragma once
#include "MaterialData.h"
#include "Types.h"

struct STextureNames
{
	inline static const string Default = "white1x1";
	inline static const string Debug = "debug";
	inline static const string White = "white1x1";
	inline static const string Water = "debug";
};

struct SMaterialSurfaces
{
	inline static SMaterialSurface Gold = {
		DirectX::XMFLOAT4(1.0f, 0.765f, 0.336f, 1.0f), // Gold color
		DirectX::XMFLOAT3(0.85f, 0.65f, 0.45f), // High reflectance
		0.1f // Smooth surface
	};
	inline static SMaterialSurface Silver = {
		DirectX::XMFLOAT4(0.972f, 0.960f, 0.915f, 1.0f), // Silver color
		DirectX::XMFLOAT3(0.95f, 0.93f, 0.88f), // High reflectance
		0.2f // Relatively smooth
	};
	inline static SMaterialSurface Bronze = {
		DirectX::XMFLOAT4(0.804f, 0.498f, 0.196f, 1.0f), // Bronze color
		DirectX::XMFLOAT3(0.65f, 0.48f, 0.36f), // Moderate reflectance
		0.2f // Relatively smooth
	};
	inline static SMaterialSurface Lambertian = {
		DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), // Neutral gray
		DirectX::XMFLOAT3(0.03f, 0.03f, 0.03f), // Low reflectance
		1.0f // Completely rough
	};
	inline static SMaterialSurface Metallic = {
		DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), // Neutral metallic color
		DirectX::XMFLOAT3(0.85f, 0.85f, 0.85f), // High reflectance
		0.1f // Smooth surface
	};
	inline static SMaterialSurface Mirror = {
		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // Perfect mirror (color does not matter much here)
		DirectX::XMFLOAT3(0.98f, 0.98f, 0.98f), // Very high reflectance
		0.0f // Perfectly smooth
	};
	inline static SMaterialSurface Glass = {
		DirectX::XMFLOAT4(0.6f, 0.7f, 0.8f, 0.1f), // Light blue, semi-transparent
		DirectX::XMFLOAT3(0.9f, 0.9f, 0.9f), // High reflectance
		0.05f // Very smooth, slightly rough to simulate imperfections
	};
	inline static SMaterialSurface Water = {
		DirectX::XMFLOAT4(0.2f, 0.4f, 0.6f, 0.5f), // Light blue, semi-transparent
		DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f), // Low reflectance
		0.8f // Slightly rough for water ripples
	};
};