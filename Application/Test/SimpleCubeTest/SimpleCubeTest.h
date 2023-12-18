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

struct SVertexPosColor
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
};

struct SObjectConstants
{
	DirectX::XMFLOAT4X4 ModelViewProj = Utils::Identity4x4();
};

class OSimpleCubeTest : public OTest
{
	using Super = OTest;

public:
	OSimpleCubeTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window);

	void LoadContent() override;
	void UnloadContent() override;

	void OnUpdate(const UpdateEventArgs& Event) override;
	void OnRender(const UpdateEventArgs& Event) override;
	void OnResize(const ResizeEventArgs& Event) override;

	void OnMouseWheel(const MouseWheelEventArgs& Event) override;
	void OnKeyPressed(const KeyEventArgs& Event) override;
	void OnMouseMoved(const MouseMotionEventArgs& Args) override;

private:
	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList,
	                          ID3D12Resource** pDestinationResource, ID3D12Resource** IntermediateResource,
	                          size_t NumElements, size_t ElementSize, const void* BufferData,
	                          D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE) const;

	void ResizeDepthBuffer(int Width, int Height);

	void CompileShader(const WCHAR* FileName, const char* EntryPoint, const char* Target,
	                   ID3DBlob** Blob) const;

	unique_ptr<OUploadBuffer<SObjectConstants>> ObjectCB = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;

	// Root signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	// Pipeline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;

	DirectX::XMFLOAT4X4 ModelMatix = Utils::Identity4x4();

	bool ContentLoaded = false;

	float Theta = 1.5f * DirectX::XM_PI;
	float Phi = DirectX::XM_PIDIV4;
	float Radius = 5.0f;
};