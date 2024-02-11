#pragma once

#include "../../../Objects/Geometry/GPUWave/GpuWave.h"
#include "../../../Objects/Geometry/Wave/Waves.h"
#include "../../Window/Window.h"
#include "../Test.h"
#include "..\..\..\Utils\DirectX.h"
#include "..\..\..\Utils\Math.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <windows.h>
#include <wrl/client.h>

class OTextureWaves : public OTest
{
	using Super = OTest;

public:
	OTextureWaves(const shared_ptr<OWindow>& _Window);

	bool Initialize() override;

	void UnloadContent() override;

	void UpdateWave(const STimer& Timer) const;

	void OnUpdate(const UpdateEventArgs& Event) override;

	void OnRender(const UpdateEventArgs& Event) override;

	void DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList,
	                     const vector<SRenderItem*>& RenderItems) const;

	OGPUWave* Waves = nullptr;

private:
	void BuildTreeSpriteGeometry();
	void BuildQuadPatchGeometry();
	void BuildTesselationPSO();
	void BuildPSOTreeSprites();

	void BuildPSOGeosphere();

	void BuildMaterials();

	void UpdateMaterialCB();

	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildWavesGeometryBuffers();
	void BuildBoxGeometryBuffers();
	void BuildRenderItems();
	void BuildIcosahedronGeometry();

	float GetHillsHeight(float X, float Z) const;

	void AnimateMaterials(const STimer& Timer);

	DirectX::XMFLOAT3 GetHillsNormal(float X, float Z) const;

	SRenderItem* Geosphere = nullptr;
	DirectX::XMFLOAT3 GeospherePos = { 0, 0, 5 };

	unique_ptr<OUploadBuffer<SVertex>> WavesVB = nullptr;

	bool ContentLoaded = false;

	vector<D3D12_INPUT_ELEMENT_DESC> TreeSpriteInputLayout;

	float Theta = 1.5f * DirectX::XM_PI;
	float Phi = DirectX::XM_PIDIV4;
	float Radius = 150.f;

	uint32_t PSOMode = 0;
	float SunTheta = 1.25f * DirectX::XM_PI;
	float SunPhi = DirectX::XM_PIDIV4;

	SRenderItem* WavesRenderItem = nullptr;
	ComPtr<ID3D12DescriptorHeap> SamplerDescriptorHeap = nullptr;

	bool IsInputBlocked = false;
};
