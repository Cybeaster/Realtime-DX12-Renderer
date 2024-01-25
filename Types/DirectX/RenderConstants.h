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
	inline static const string Shadow = "Shadow";
	inline static const string Reflected = "Reflected";
	inline static const string Mirror = "Mirror";
	inline static const string AlphaTestedTreeSprites = "AlphaTestedTreeSprites";
	inline static const string IcosahedronLODs = "IcosahedronLODs";
};

struct SPSOType
{
	inline static const string Opaque = "Opaque";
	inline static const string Transparent = "Transparent";
	inline static const string AlphaTested = "AlphaTested";
	inline static const string Shadow = "Shadow";
	inline static const string Debug = "Debug";
	inline static const string StencilReflection = "Reflected";
	inline static const string StencilMirrors = "Mirror";
	inline static const string TreeSprites = "TreeSprites";
	inline static const string Icosahedron = "Icosahedron";
	inline static const string HorizontalBlur = "HorizontalBlur";
	inline static const string VerticalBlur = "VerticalBlur";
	inline static const string SobelFilter = "SobelFilter";
	inline static const string Composite = "Composite";
};

struct SShaderTypes
{
	inline static const string VSBaseShader = "VSBaseShader";

	inline static const string PSAlphaTested = "PSAlphaTested";
	inline static const string PSOpaque = "PSOpaque";
	inline static const string PSBaseShader = "PSBaseShader";

	inline static const string VSTreeSprite = "VSTreeSprite";
	inline static const string GSTreeSprite = "GSTreeSprite";
	inline static const string PSTreeSprite = "PSTreeSprite";

	inline static const string GSIcosahedron = "GSIcosahedron";
	inline static const string PSIcosahedron = "PSIcosahedron";
	inline static const string VSIcosahedron = "VSIcosahedron";

	inline static const string CSHorizontalBlur = "CSHorizontalBlur";
	inline static const string CSVerticalBlur = "CSVerticalBlur";

	inline static const string PSComposite = "PSComposite";
	inline static const string VSComposite = "VSComposite";

	inline static const string CSWavesDisturb = "CSWavesDisturb";
	inline static const string CSWavesUpdate = "CSWavesUpdate";

	inline static const string CSSobelFilter = "CSSobelFilter";
};
