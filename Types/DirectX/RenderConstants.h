#pragma once
#include "DXHelper.h"

struct SRenderConstants
{
	struct SSphere
	{
		const uint32_t SliceCount = 20;
		const uint32_t StackCount = 20;
		const float Radius = 0.5f;
	};

	struct SCylinder
	{
		const float BottomRadius = 1.f;
		const float TopRadius = 0.5f;
		const float Height = 3.0f;
		const uint32_t SliceCount = 20;
		const uint32_t StackCount = 20;
	};

	inline static constexpr D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	inline static constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	inline static constexpr DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	inline static constexpr uint32_t NumFrameResources = 3;
	inline static constexpr uint32_t MaxLights = 16;
	inline static constexpr uint32_t RenderBuffersCount = 2;
	inline static constexpr DirectX::XMUINT2 CubeMapDefaultResolution = { 1024, 1024 };

	inline static constexpr uint32_t MaxDiffuseMapsPerMaterial = 3;
	inline static constexpr uint32_t MaxNormalMapsPerMaterial = 3;
	inline static constexpr uint32_t MaxHeightMapsPerMaterial = 3;

	inline static constexpr SSphere Sphere;
	inline static constexpr SCylinder Cylinder;
	inline static const string DefaultSkyTexture = "grasscube1024";
};

struct SRenderLayer
{
	RENDER_TYPE(Opaque);
	RENDER_TYPE(OpaqueDynamicReflections);
	RENDER_TYPE(Transparent);
	RENDER_TYPE(AlphaTested);
	RENDER_TYPE(Shadow);
	RENDER_TYPE(Reflected);
	RENDER_TYPE(Mirror);
	RENDER_TYPE(AlphaTestedTreeSprites);
	RENDER_TYPE(IcosahedronLODs);
	RENDER_TYPE(Waves);
	RENDER_TYPE(Tesselation);
	RENDER_TYPE(Highlight);
	RENDER_TYPE(Sky);
	RENDER_TYPE(PostProcess);
	RENDER_TYPE(Water);
};

struct SPSOType
{
	RENDER_TYPE(Opaque);
	RENDER_TYPE(Transparent);
	RENDER_TYPE(AlphaTested);
	RENDER_TYPE(Shadow);
	RENDER_TYPE(Debug);
	RENDER_TYPE(StencilReflection);
	RENDER_TYPE(StencilMirrors);
	RENDER_TYPE(TreeSprites);
	RENDER_TYPE(Icosahedron);
	RENDER_TYPE(HorizontalBlur);
	RENDER_TYPE(VerticalBlur);
	RENDER_TYPE(SobelFilter);
	RENDER_TYPE(Composite);

	RENDER_TYPE(WavesRender);
	RENDER_TYPE(WavesDisturb);
	RENDER_TYPE(WavesUpdate);

	RENDER_TYPE(BilateralBlur);
	RENDER_TYPE(Tesselation);
	RENDER_TYPE(Highlight);
	RENDER_TYPE(Sky);
};
struct SShaderTypes
{
	RENDER_TYPE(VSBaseShader);

	RENDER_TYPE(PSAlphaTested);
	RENDER_TYPE(PSOpaque);
	RENDER_TYPE(PSBaseShader);

	RENDER_TYPE(VSTreeSprite);
	RENDER_TYPE(GSTreeSprite);
	RENDER_TYPE(PSTreeSprite);

	RENDER_TYPE(GSIcosahedron);
	RENDER_TYPE(PSIcosahedron);
	RENDER_TYPE(VSIcosahedron);

	RENDER_TYPE(CSHorizontalBlur);
	RENDER_TYPE(CSVerticalBlur);

	RENDER_TYPE(CSBilateralBlur);

	RENDER_TYPE(PSComposite);
	RENDER_TYPE(VSComposite);

	RENDER_TYPE(CSWavesDisturb);
	RENDER_TYPE(CSWavesUpdate);
	RENDER_TYPE(VSWaves);

	RENDER_TYPE(CSSobelFilter);
	RENDER_TYPE(VSTesselation);
	RENDER_TYPE(HSTesselation);
	RENDER_TYPE(DSTesselation);
	RENDER_TYPE(PSTesselation);
	RENDER_TYPE(VSSky);
	RENDER_TYPE(PSSky);
};
