#pragma once

#include "..\..\..\Utils\DirectX.h"
#include "..\..\..\Utils\Math.h"
#include "../../Window/Window.h"
#include "../Test.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"
#include "../../../Objects/Geometry/Wave/Waves.h"
#include "Textures/Texture.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <windows.h>
#include <wrl/client.h>

class OStencilingTest : public OTest
{
	using Super = OTest;

	struct
	{
		static constexpr string BricksTexture = "BricksTexture";
		static constexpr string CheckboardTexture = "CheckboardTexture";
		static constexpr string IceTexture = "IceTexture";
		static constexpr string White1x1Texture = "White1x1Texture";
	};

public:
	OStencilingTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window);

	bool Initialize() override;

	void UnloadContent() override;

	void OnUpdate(const UpdateEventArgs& Event) override;

	void OnRender(const UpdateEventArgs& Event) override;

	void OnResize(const ResizeEventArgs& Event) override;

	void OnKeyPressed(const KeyEventArgs& Event) override;

	void OnMouseMoved(const MouseMotionEventArgs& Args) override;

	void UpdateMainPass(const STimer& Timer);

	void UpdateObjectCBs(const STimer& Timer);

	void DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList,
	                     const vector<SRenderItem*>& RenderItems) const;

	void UpdateCamera();

	void OnKeyboardInput(const STimer& Timer);

	void OnMouseWheel(const MouseWheelEventArgs& Args) override;

private:
	void CreateTexture();

	void SetupProjection();

	void BuildMaterials();

	void UpdateMaterialCB();

	void BuildRootSignature();

	void BuildShadersAndInputLayout();

	void BuildDescriptorHeap();
	void BuildRoomGeomety();
	void BuildRenderItems();

	float GetHillsHeight(float X, float Z) const;

	void AnimateMaterials(const STimer& Timer);

	DirectX::XMFLOAT3 GetHillsNormal(float X, float Z) const;


	ComPtr<ID3D12RootSignature> RootSignature;


	SPassConstants MainPassCB;

	unique_ptr<OUploadBuffer<SVertex>> WavesVB = nullptr;
	DirectX::XMFLOAT3 EyePos = { 0, 0, 0 };
	DirectX::XMFLOAT4X4 ViewMatrix = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 ProjectionMatrix = Utils::Math::Identity4x4();

	bool ContentLoaded = false;

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	float Theta = 1.5f * DirectX::XM_PI;
	float Phi = DirectX::XM_PIDIV4;
	float Radius = 150.f;

	uint32_t PSOMode = 0;
	float SunTheta = 1.25f * DirectX::XM_PI;
	float SunPhi = DirectX::XM_PIDIV4;

	SRenderItem* WavesRenderItem = nullptr;
	ComPtr<ID3D12DescriptorHeap> SamplerDescriptorHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> SRVHeap;
};
