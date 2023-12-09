
#include "SimpleCubeTest.h"

#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"

#include <DXHelper.h>

#include <filesystem>
#include <iostream>
using namespace Microsoft::WRL;

using namespace DirectX;

struct SVertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

static SVertexPosColor Vertices[8] = {
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) } // 7
};

struct SPipelineStateStream
{
	CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
	CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
	CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
	CD3DX12_PIPELINE_STATE_STREAM_VS VS;
	CD3DX12_PIPELINE_STATE_STREAM_PS PS;
	CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
	CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
} PipelineStateStream;

static WORD Indices[36] = {
	0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7
};

OSimpleCubeTest::OSimpleCubeTest(const shared_ptr<OEngine>& _Engine)
    : OTest(_Engine), FenceValues{ _Engine->GetWindow()->BuffersCount }, Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, _Engine->GetWidth(), _Engine->GetHeight())), ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)), FoV(45.0), ContentLoaded(false)
{
}

void OSimpleCubeTest::LoadContent()
{
	auto device = Engine.lock()->GetDevice();
	const auto commandQueue = Engine.lock()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	ComPtr<ID3D12Resource> intermediateVertexBuffer;
	UpdateBufferResource(commandList, &VertexBuffer, &intermediateVertexBuffer, _countof(Vertices), sizeof(SVertexPosColor), Vertices);

	D3D12_RESOURCE_BARRIER vertexBufferBarrier = {};
	vertexBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	vertexBufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	vertexBufferBarrier.Transition.pResource = VertexBuffer.Get();
	vertexBufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	vertexBufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	vertexBufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &vertexBufferBarrier);


	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
	VertexBufferView.SizeInBytes = _countof(Vertices) * sizeof(SVertexPosColor); // Correct size calculation
	VertexBufferView.StrideInBytes = sizeof(SVertexPosColor);

	ComPtr<ID3D12Resource> intermediateIndexBuffer;
	UpdateBufferResource(commandList, &IndexBuffer, &intermediateIndexBuffer, _countof(Indices), sizeof(WORD), Indices);

	// After updating the index buffer
	D3D12_RESOURCE_BARRIER indexBufferBarrier = {};
	indexBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	indexBufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	indexBufferBarrier.Transition.pResource = IndexBuffer.Get();
	indexBufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	indexBufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	indexBufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&DSVHeap)));

	// Load vertex shader
	ComPtr<ID3DBlob> vertexShaderBlob;
	CompileShader(L"../Shaders/OneSimpleCube/VertexShader.hlsl", "main", "vs_5_1", &vertexShaderBlob);

	// Load the pixel shader
	ComPtr<ID3DBlob> pixelShaderBlob;
	CompileShader(L"../Shaders/OneSimpleCube/PixelShader.hlsl", "main", "ps_5_1", &pixelShaderBlob);

	D3D12_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDescription;
	rootSigDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// Serialize the root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSigDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	PipelineStateStream.pRootSignature = RootSignature.Get();
	PipelineStateStream.InputLayout = { inputDesc, _countof(inputDesc) };
	PipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	PipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	PipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	PipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(SPipelineStateStream), &PipelineStateStream
	};
	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&PipelineState)));

	const auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);
	ContentLoaded = true;
}

void OSimpleCubeTest::UnloadContent()
{
	ContentLoaded = false;
}

void OSimpleCubeTest::OnUpdate(UpdateEventArgs& Event)
{
	static uint64_t frameCount = 0;
	static double totalTime = 0.0;

	Super::OnUpdate(Event);
	totalTime += Event.ElapsedTime;

	if (totalTime > 1.0)
	{
		double fps = frameCount / totalTime;

		char buffer[512];
		sprintf_s(buffer, "Simple Cube Test [FPS: %f]", fps);

		totalTime = 0;
		frameCount = 0;
	}

	float angle = static_cast<float>(Event.TotalTime * 90.0);
	const XMVECTOR rotAxis = XMVectorSet(0, 1, 1, 0);
	ModelMatix = XMMatrixRotationAxis(rotAxis, XMConvertToRadians(angle));

	const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
	const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	const float aspectRatio = Engine.lock()->GetWidth() / static_cast<float>(Engine.lock()->GetHeight());
	ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(FoV), aspectRatio, 0.1f, 100.0f);
}

void OSimpleCubeTest::OnRender()
{
	auto commandQueue = Engine.lock()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList();

	auto window = Engine.lock()->GetWindow();

	UINT currentBackBufferIndex = Engine.lock()->GetWindow()->GetCurrentBackBufferIndex();
	auto backBuffer = Engine.lock()->GetWindow()->GetCurrentBackBuffer();
	auto rtv = window->GetCurrentRenderTargetView();
	auto svc = DSVHeap->GetCPUDescriptorHandleForHeapStart();

	// clear render targets
	{
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

		ClearRTV(commandList, rtv, clearColor);
		ClearDepth(commandList, svc);
	}

	commandList->SetPipelineState(PipelineState.Get());
	commandList->SetGraphicsRootSignature(RootSignature.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	commandList->IASetIndexBuffer(&IndexBufferView);

	commandList->RSSetViewports(1, &Viewport);
	commandList->RSSetScissorRects(1, &ScissorRect);

	commandList->OMSetRenderTargets(1, &rtv, FALSE, &svc);

	XMMATRIX mvp = ModelMatix * ViewMatrix * ProjectionMatrix;
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);
	commandList->DrawIndexedInstanced(_countof(Indices), 1, 0, 0, 0);

	// present
	{
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
		currentBackBufferIndex = window->Present();
		commandQueue->WaitForFenceValue(FenceValues[currentBackBufferIndex]);
	}
}

void OSimpleCubeTest::OnResize(ResizeEventArgs& Event)
{
	if (Event.Width != Engine.lock()->GetWidth() || Event.Height != Engine.lock()->GetHeight())
	{
		OTest::OnResize(Event);

		Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Event.Width), static_cast<float>(Event.Height));
		ResizeDepthBuffer(Event.Width, Event.Height);
	}
}

void OSimpleCubeTest::OnMouseWheel(MouseWheelEventArgs& Event)
{
	FoV -= Event.WheelDelta;
	FoV = std::clamp(FoV, 12.0f, 90.0f);

	char buffer[256];
	sprintf_s(buffer, "FoV: %f\n", FoV);
	OutputDebugStringA(buffer);
}

void OSimpleCubeTest::OnKeyPressed(KeyEventArgs& Event)
{
	OTest::OnKeyPressed(Event);

	switch (Event.Key)
	{
	case KeyCode::Escape:
		OApplication::Get()->Quit(0);
		break;
	case KeyCode::Enter:
		if (Event.Alt)
		{
		case KeyCode::F11:
			Engine.lock()->GetWindow()->ToggleFullscreen();
			break;
		}
	case KeyCode::V:
		Engine.lock()->GetWindow()->ToggleVSync();
		break;
	}
}

void OSimpleCubeTest::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> CommandList, ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState)
{
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), BeforeState, AfterState);
	CommandList->ResourceBarrier(1, &barrier);
}

void OSimpleCubeTest::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor)
{
	CommandList->ClearRenderTargetView(RTV, ClearColor, 0, nullptr);
}

void OSimpleCubeTest::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth)
{
	CommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, Depth, 0, 0, nullptr);
}

void OSimpleCubeTest::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
                                           ID3D12Resource** DestinationResource,
                                           ID3D12Resource** IntermediateResource,
                                           size_t NumElements,
                                           size_t ElementSize,
                                           const void* BufferData,
                                           D3D12_RESOURCE_FLAGS Flags) const
{
	auto device = Engine.lock()->GetDevice();
	size_t bufferSize = NumElements * ElementSize;

	// Create named instances for heap properties and resource description
	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, Flags);

	// Create a committed resource for the GPU resource in a default heap.
	ThrowIfFailed(device->CreateCommittedResource(
	    &defaultHeapProperties,
	    D3D12_HEAP_FLAG_NONE,
	    &bufferResourceDesc,
	    D3D12_RESOURCE_STATE_COMMON,
	    nullptr,
	    IID_PPV_ARGS(DestinationResource)));

	if (BufferData)
	{
		// Create named instances for upload heap properties and resource description
		const CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		// Create a committed resource for the upload.
		ThrowIfFailed(device->CreateCommittedResource(
		    &uploadHeapProperties,
		    D3D12_HEAP_FLAG_NONE,
		    &uploadBufferResourceDesc,
		    D3D12_RESOURCE_STATE_GENERIC_READ,
		    nullptr,
		    IID_PPV_ARGS(IntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = BufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(CommandList.Get(), *DestinationResource, *IntermediateResource, 0, 0, 1, &subresourceData);
	}
}

void OSimpleCubeTest::ResizeDepthBuffer(int Width, int Height)
{
	if (ContentLoaded)
	{
		// Flush any GPU commands that might be referencing the depth buffer.
		auto engine = Engine.lock();
		engine->FlushGPU();
		Width = std::max(1, Width);
		Height = std::max(1, Height);

		const auto device = engine->GetDevice();

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		// Create named instances for heap properties and resource description
		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		ThrowIfFailed(device->CreateCommittedResource(
		    &heapProperties,
		    D3D12_HEAP_FLAG_NONE,
		    &resourceDesc,
		    D3D12_RESOURCE_STATE_DEPTH_WRITE,
		    &optimizedClearValue,
		    IID_PPV_ARGS(&DepthBuffer)));

		// Depth stencil view description
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(DepthBuffer.Get(), &dsvDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

void OSimpleCubeTest::CompileShader(const WCHAR* FileName, const char* EntryPoint, const char* Target, ID3DBlob** Blob) const
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
	HRESULT hr = D3DCompileFromFile(FileName, nullptr, nullptr, EntryPoint, Target, dwShaderFlags, 0, Blob, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}
		THROW_IF_FAILED(hr);
	}
}
