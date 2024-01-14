#pragma once
#include "DXHelper.h"

struct SRenderConstants
{
	inline static constexpr D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	inline static constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	inline static constexpr DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	inline static constexpr uint32_t NumFrameResources = 3;
	inline static constexpr uint32_t MaxLights = 16;
};


struct SRenderLayer
{
	inline static const string Opaque = "Opaque";
	inline static const string Transparent = "Transparent";
	inline static const string AlphaTested = "AlphaTested";
};

struct SPSOType
{
	inline static const string Opaque = "Opaque";
	inline static const string Transparent = "Transparent";
	inline static const string AlphaTested = "AlphaTested";
	inline static const string Shadow = "Shadow";
	inline static const string Debug = "Debug";
};

struct SShaderTypes
{
	inline static const string VSBaseShader = "VSBaseShader";
	inline static const string PSAlphaTested = "PSAlphaTested";
	inline static const string PSOpaque = "PSOpaque";
	inline static const string PSBaseShader = "PSBaseShader";
};
