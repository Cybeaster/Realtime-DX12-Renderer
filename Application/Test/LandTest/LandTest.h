#pragma once

#include "../../../Utils/DXUtils.h"
#include "../../../Utils/MathUtils.h"
#include "../../Window/Window.h"
#include "../Test.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"
#include "../../../Objects/Geometry/Wave/Waves.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <windows.h>
#include <wrl/client.h>

class OLandTest : public OTest
{
	using Super = OTest;

public:
	OLandTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window);

	bool Initialize() override;

	void UnloadContent() override;

	void UpdateWave(const STimer& Timer);

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

	void OnKeyboardInput();

private:
	void SetupProjection();

	void BuildDescriptorHeaps();

	void BuildConstantBuffersViews();

	void BuildRootSignature();

	void BuildShadersAndInputLayout();

	void BuildLandGeometry();

	void BuildWavesGeometryBuffers();

	void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
	                          ID3D12Resource** pDestinationResource, ID3D12Resource** IntermediateResource,
	                          size_t NumElements, size_t ElementSize, const void* BufferData,
	                          D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE) const;

	void BuildPSO();

	void BuildRenderItems();

	float GetHillsHeight(float X, float Z) const;

	DirectX::XMFLOAT3 GetHillsNormal(float X, float Z) const;

	ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;

	ComPtr<ID3D12RootSignature> RootSignature;

	unique_ptr<SMeshGeometry> BoxGeometry;
	unique_ptr<OUploadBuffer<SObjectConstants>> ObjectCB = nullptr;
	unique_ptr<OUploadBuffer<STimerConstants>> ObjectCBTime = nullptr;

	SPassConstants MainPassCB;

	ComPtr<ID3D12DescriptorHeap> CBVHeap = nullptr;
	unique_ptr<OUploadBuffer<SVertex>> WavesVB = nullptr;
	DirectX::XMFLOAT3 EyePos = { 0, 0, 0 };
	DirectX::XMFLOAT4X4 ViewMatrix = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 ProjectionMatrix = Utils::Math::Identity4x4();

	bool ContentLoaded = false;

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	float Theta = 1.5f * DirectX::XM_PI;
	float Phi = DirectX::XM_PIDIV4;
	float Radius = 35;

	UINT PassConstantCBVOffset = 0;
	bool bIsWireFrame = false;

	SRenderItem* WavesRenderItem = nullptr;
};
