#pragma once
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "HlslTypes.h"
#include "Logger.h"
#include "ObjectConstants.h"
#include "Vertex.h"

#include <Types.h>
#include <d3d12.h>
#include <wrl/client.h>

struct SSsaoConstants
{
	DirectX::XMFLOAT4X4 Proj;
	DirectX::XMFLOAT4X4 InvProj;
	DirectX::XMFLOAT4X4 ProjTex;
	DirectX::XMFLOAT4 OffsetVectors[14] = {};

	// For SsaoBlur.DisneyBRDF.hlsl
	DirectX::XMFLOAT4 BlurWeights[3] = {};

	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };

	// Coordinates given in view space.
	float OcclusionRadius = 0.5f;
	float OcclusionFadeStart = 0.2f;
	float OcclusionFadeEnd = 2.0f;
	float SurfaceEpsilon = 0.05f;
};

struct SFrameResource
{
	SFrameResource(ID3D12Device* Device, IRenderObject* Owner);

	SFrameResource(const SFrameResource&) = delete;

	SFrameResource& operator=(const SFrameResource&) = delete;

	~SFrameResource();

	// We cannot update a cbuffer until the GPU is done processing the
	// commands that reference it. So each frame needs their own cbuffers.

	void SetPass(UINT PassCount);
	void SetCameraInstances(UINT InstanceCount);
	void SetMaterials(UINT MaterialCount);
	void SetDirectionalLight(UINT LightCount);
	void SetPointLight(UINT LightCount);
	void SetSpotLight(UINT LightCount);
	void SetSSAO();
	void SetFrusturmCorners();
	TUploadBuffer<SPassConstants> PassCB = nullptr;
	TUploadBuffer<HLSL::MaterialData> MaterialBuffer = nullptr;
	TUploadBuffer<HLSL::InstanceData> CameraInstancesBuffer = nullptr;
	TUploadBuffer<SSsaoConstants> SsaoCB = nullptr;
	TUploadBuffer<HLSL::DirectionalLight> DirectionalLightBuffer;
	TUploadBuffer<HLSL::PointLight> PointLightBuffer;
	TUploadBuffer<HLSL::SpotLight> SpotLightBuffer;
	TUploadBuffer<HLSL::FrustrumCorners> FrusturmCornersBuffer;
	TUploadBuffer<HLSL::CameraMatrixBuffer> CameraMatrixBuffer;
	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
	IRenderObject* Owner = nullptr;
	ID3D12Device* Device = nullptr;
};