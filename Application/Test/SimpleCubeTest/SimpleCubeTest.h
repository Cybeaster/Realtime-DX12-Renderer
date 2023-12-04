#pragma once

#include "../Test.h"
#include "Events.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <windows.h>
#include <wrl/client.h>

class OSimpleCubeTest : public OTest
{
	using Super = OTest;

public:
	OSimpleCubeTest(const shared_ptr<class OEngine>& _Engine);

	void LoadContent() override;
	void UnloadContent() override;

	void Destroy() override;
	void Update() override;

	void OnUpdate(UpdateEventArgs& Event) override;
	void OnRender() override;
	void OnResize(ResizeEventArgs& Event) override;

	void OnMouseWheel(MouseMotionEventArgs& Event) override;

private:
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList,
	                        Microsoft::WRL::ComPtr<ID3D12Resource> Resource,
	                        D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState);

	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList,
	              D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor);

	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList,
	                D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth = 1.0f);

	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList,
	                          ID3D12Resource** pDestinationResource, ID3D12Resource** IntermediateResource,
	                          size_t NumElements, size_t ElementSize, const void* BufferData,
	                          D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE) const;

	void ResizeDepthBuffer(int Width, int Height);

	uint64_t FenceValue[];

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;

	// Depth buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> DepthBuffer;

	// Descriptor heap for depth buffer
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap;

	// Root signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	// Pipeline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	float FoV;

	DirectX::XMMATRIX ModelMatix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;

	bool ContentLoaded;
};