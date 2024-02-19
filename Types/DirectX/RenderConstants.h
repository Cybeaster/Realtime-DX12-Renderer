#pragma once
#include "DXHelper.h"

#define RENDER_TYPE inline static const string
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
	RENDER_TYPE Opaque = "Opaque";
	RENDER_TYPE Transparent = "Transparent";
	RENDER_TYPE AlphaTested = "AlphaTested";
	RENDER_TYPE Shadow = "Shadow";
	RENDER_TYPE Reflected = "Reflected";
	RENDER_TYPE Mirror = "Mirror";
	RENDER_TYPE AlphaTestedTreeSprites = "AlphaTestedTreeSprites";
	RENDER_TYPE IcosahedronLODs = "IcosahedronLODs";
	RENDER_TYPE Waves = "Waves";
	RENDER_TYPE Tesselation = "Tesselation";
	RENDER_TYPE Highlight = "Highlight";
};

struct SPSOType
{
	RENDER_TYPE Opaque = "Opaque";
	RENDER_TYPE Transparent = "Transparent";
	RENDER_TYPE AlphaTested = "AlphaTested";
	RENDER_TYPE Shadow = "Shadow";
	RENDER_TYPE Debug = "Debug";
	RENDER_TYPE StencilReflection = "Reflected";
	RENDER_TYPE StencilMirrors = "Mirror";
	RENDER_TYPE TreeSprites = "TreeSprites";
	RENDER_TYPE Icosahedron = "Icosahedron";
	RENDER_TYPE HorizontalBlur = "HorizontalBlur";
	RENDER_TYPE VerticalBlur = "VerticalBlur";
	RENDER_TYPE SobelFilter = "SobelFilter";
	RENDER_TYPE Composite = "Composite";

	RENDER_TYPE WavesRender = "WavesRender";
	RENDER_TYPE WavesDisturb = "WavesDisturb";
	RENDER_TYPE WavesUpdate = "WavesUpdate";

	RENDER_TYPE BilateralBlur = "BilateralBlur";
	RENDER_TYPE Tesselation = "Tesselation";
	RENDER_TYPE Highlight = "Highlight";
};

struct SShaderTypes
{
	RENDER_TYPE VSBaseShader = "VSBaseShader";

	RENDER_TYPE PSAlphaTested = "PSAlphaTested";
	RENDER_TYPE PSOpaque = "PSOpaque";
	RENDER_TYPE PSBaseShader = "PSBaseShader";

	RENDER_TYPE VSTreeSprite = "VSTreeSprite";
	RENDER_TYPE GSTreeSprite = "GSTreeSprite";
	RENDER_TYPE PSTreeSprite = "PSTreeSprite";

	RENDER_TYPE GSIcosahedron = "GSIcosahedron";
	RENDER_TYPE PSIcosahedron = "PSIcosahedron";
	RENDER_TYPE VSIcosahedron = "VSIcosahedron";

	RENDER_TYPE CSHorizontalBlur = "CSHorizontalBlur";
	RENDER_TYPE CSVerticalBlur = "CSVerticalBlur";

	RENDER_TYPE CSBilateralBlur = "CSBilateralBlur";

	RENDER_TYPE PSComposite = "PSComposite";
	RENDER_TYPE VSComposite = "VSComposite";

	RENDER_TYPE CSWavesDisturb = "CSWavesDisturb";
	RENDER_TYPE CSWavesUpdate = "CSWavesUpdate";
	RENDER_TYPE VSWaves = "PSWaves";

	RENDER_TYPE CSSobelFilter = "CSSobelFilter";
	RENDER_TYPE VSTesselation = "VSTesselation";
	RENDER_TYPE HSTesselation = "HSTesselation";
	RENDER_TYPE DSTesselation = "DSTesselation";
	RENDER_TYPE PSTesselation = "PSTesselation";
};
