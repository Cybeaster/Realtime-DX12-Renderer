
#ifndef HLSL
#pragma once
#define float3 DirectX::XMFLOAT3
#define float4 DirectX::XMFLOAT4
#define float2 DirectX::XMFLOAT2
#define float4x4 DirectX::XMFLOAT4X4
#define uint unsigned int

#include "DirectX/DXHelper.h"
#include "MathUtils.h"
namespace HLSL
{
#endif

#ifndef HLSL
#define MAKE_CBUFFER(Name, Register, ...) struct Name
#else
#define MAKE_CBUFFER(Name, Register) cbuffer Name : register(Register)
#endif

#ifndef MAX_SHADOW_MAPS
#define MAX_SHADOW_MAPS 4
#endif

#define TEXTURE_MAPS_NUM 312
#define SHADOW_MAPS_NUM 1

#define EPSILON 0.0001f
#define SHADOW_EPSILON 0.0005f
#define F0_COEFF 0.16f
#define MAX_CSM_PER_FRAME 3

#define STRINGIFY(x) #x
#define STRINGIFY_MACRO(x) STRINGIFY(x)
#define CB_SSAO cbSsao
#define CB_PASS cbPass
#define TLAS gTLAS
#define CB_ROOT_CONSTANTS cbRootConstants
#define CUBE_MAP gCubeMap
#define TEXTURE_MAPS gTextureMaps
#define MATERIAL_DATA gMaterialData
#define DIRECTIONAL_LIGHTS gDirectionalLights
#define POINT_LIGHTS gPointLights
#define SPOT_LIGHTS gSpotLights
#define SHADOW_MAPS gShadowMaps
#define SSAO_MAP gSsaoMap
#define FRUSTRUM_CORNERS gFrustrumCorners
#define CAMERA_MATRIX cbCameraMatrix
#define AABBData gAABBData
#define INSTANCE_DATA gInstanceData
#define NORMAL_MAP gNormalMap
#define RANDOM_VEC_MAP gRandomVecMap
#define DEPTH_MAP gDepthMap
#define OUTPUT gOutput
struct TextureData
{
	uint bIsEnabled;
	uint TextureIndex;
};

struct MaterialData
{
	float3 AmbientAlbedo;
	float Shininess;

	float3 DiffuseAlbedo;
	float IndexOfRefraction;

	float3 SpecularAlbedo;
	float Dissolve;

	float3 Transmittance;
	int Illumination;

	float3 Emission;
	float Roughness;

	float Metalness;
	float Sheen;
	float Reflection;
	float pad;
	float4x4 MatTransform;
	TextureData DiffuseMap;
	TextureData NormalMap;
	TextureData HeightMap;
	TextureData AlphaMap;
	TextureData SpecularMap;
	TextureData AmbientMap;
};

struct InstanceData
{
#ifndef HLSL
	InstanceData()
	{
		DirectX::XMStoreFloat4x4(&World, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&TexTransform, DirectX::XMMatrixIdentity());
		Position = { 0.0f, 0.0f, 0.0f };
		Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		Scale = { 1.0f, 1.0f, 1.0f };
		MaterialIndex = 0;
		BoundingBoxCenter = { 0.0f, 0.0f, 0.0f };
		BoundingBoxExtents = { 0.0f, 0.0f, 0.0f };
	}

	bool IsTransformEqual(const InstanceData& Other) const
	{
		return Other.Position == Position && Other.Rotation == Rotation && Other.Scale == Scale;
	}
#endif

	float4x4 World;
	float4x4 TexTransform;
	uint MaterialIndex;
	float3 Position;
	float pad;
	float4 Rotation;
	float3 Scale;
	float pad3;
	float3 BoundingBoxCenter;
	float pad4;
	float3 BoundingBoxExtents;
	float pad5;
	float3 OverrideColor;
	float pad6;
	float4x4 InvViewProjection;
};

struct SpotLight
{
	float3 Position;
	float pad1; // Padding to align with HLSL's float4
	float3 Direction;
	float pad2; // Additional padding if necessary
	float3 Intensity;
	float FalloffStart;
	float FalloffEnd;
	float SpotPower;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float ConeAngle;
	float4x4 Transform;
};

struct PointLight
{
	float3 Position;
	float pad1; // Padding to align with HLSL's float4
	float3 Intensity;
	float FalloffStart;
	float FalloffEnd;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float2 pad2; // Padding to align with HLSL's float4
	float4x4 Transform;
};

struct CameraMatrixBuffer
{
	float4x4 gCamViewProj;
	float4x4 gCamInvViewProj;
};

//supposed to use cascade shadow mapping

struct ShadowMapData
{
	float4x4 Transform;
	uint ShadowMapIndex; // Index to the shadow map texture array
	float3 pad; // Padding to align with HLSL's float4
};

struct DirectionalLight
{
	float3 Direction;
	float pad1; // Padding to align with HLSL's float4
	float4 QuatRotaion;
	float3 Intensity;
	float pad2;
	ShadowMapData ShadowMapData[MAX_CSM_PER_FRAME];
};

struct FrustrumCorners
{
	float3 Corners[8];
};

struct HitInfo
{
	float4 Color;
};

struct Attributes
{
	float2 Bary;
};

struct CameraCBuffer
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gViewProjTex;
	float3 gEyePosW;

	float FOV; // Use this to pad gEyePosW to 16 bytes

	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;
	float4 gFogColor;
	float gFogStart;
	float gFogRange;

	uint gNumDirLights;

	float AspectRatio; // Padding to align following uints
	uint gNumPointLights;
	uint gNumSpotLights;
	float FocusDistance; // Padding to ensure the cbuffer ends on a 16-byte boundary
	float Aperture; // Padding to ensure the cbuffer ends on a 16-byte boundary
	bool gSSAOEnabled;
};

#ifndef HLSL
inline float3 FrustumCornens[8] = {
	float3(-1.0f, 1.0f, -1.0f),
	float3(1.0f, 1.0f, -1.0f),
	float3(1.0f, -1.0f, -1.0f),
	float3(-1.0f, -1.0f, -1.0f),
	float3(-1.0f, 1.0f, 1.0f),
	float3(1.0f, 1.0f, 1.0f),
	float3(1.0f, -1.0f, 1.0f),
	float3(-1.0f, -1.0f, 1.0f)
};

#endif

#ifndef HLSL
}
#endif
