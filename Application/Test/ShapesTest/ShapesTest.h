#pragma once

#include "../../../Utils/DXUtils.h"
#include "../../../Utils/MathUtils.h"
#include "../../Window/Window.h"
#include "../Test.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <windows.h>
#include <wrl/client.h>

struct SVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

class OShapesTest : public OTest
{
	using Super = OTest;

public:
	OShapesTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window);
	bool Initialize() override;
	void UnloadContent() override;
	void OnUpdate(const UpdateEventArgs& Event) override;
	void OnRender(const UpdateEventArgs& Event) override;
	void OnResize(const ResizeEventArgs& Event) override;
	void OnMouseWheel(const MouseWheelEventArgs& Event) override;
	void OnKeyPressed(const KeyEventArgs& Event) override;
	void OnMouseMoved(const MouseMotionEventArgs& Args) override;

	void UpdateMainPass(STimer& Timer);
	void UpdateObjectCBs(STimer& Timer);

private:
	void SetupProjection();
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
	                          ID3D12Resource** pDestinationResource, ID3D12Resource** IntermediateResource,
	                          size_t NumElements, size_t ElementSize, const void* BufferData,
	                          D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE) const;

	void BuildPSO();

	ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;

	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12PipelineState> PipelineStateObject;

	unique_ptr<SMeshGeometry> BoxGeometry;
	unique_ptr<OUploadBuffer<SObjectConstants>> ObjectCB = nullptr;
	unique_ptr<OUploadBuffer<STimerConstants>> ObjectCBTime = nullptr;

	SPassConstants MainPassCB;

	ComPtr<ID3D12DescriptorHeap> CBVHeap = nullptr;

	DirectX::XMFLOAT3 EyePos = { 0, 0, 0 };
	DirectX::XMFLOAT4X4 ViewMatrix = Utils::Identity4x4();
	DirectX::XMFLOAT4X4 ProjectionMatrix = Utils::Identity4x4();

	bool ContentLoaded = false;

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	ComPtr<ID3DBlob> MvsByteCode = nullptr;
	ComPtr<ID3DBlob> MpsByteCode = nullptr;

	float Theta = 1.5f * DirectX::XM_PI;
	float Phi = DirectX::XM_PIDIV4;
	float Radius = 5;
};
