#pragma once

#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"
#include "Test/Test.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <windows.h>
#include <wrl/client.h>

class OGPUWave;
class OTextureWaves : public OTest
{
	using Super = OTest;

public:
	OTextureWaves(OWindow* _Window);
	bool Initialize() override;
	void OnUpdate(const UpdateEventArgs& Event) override;

	OGPUWave* Waves = nullptr;

private:
	void BuildTreeSpriteGeometry();
	void BuildQuadPatchGeometry();
	void BuildTesselationPSO();
	void BuildPSOTreeSprites();

	void BuildPSOGeosphere();
	void UpdateMaterialCB();

	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildRenderItems();

	float GetHillsHeight(float X, float Z) const;

	void AnimateMaterials(const STimer& Timer);

	DirectX::XMFLOAT3 GetHillsNormal(float X, float Z) const;

	ORenderItem* Geosphere = nullptr;
	DirectX::XMFLOAT3 GeospherePos = { 0, 0, 5 };

	unique_ptr<OUploadBuffer<SVertex>> WavesVB = nullptr;

	vector<D3D12_INPUT_ELEMENT_DESC> TreeSpriteInputLayout;

	bool IsInputBlocked = false;
};
