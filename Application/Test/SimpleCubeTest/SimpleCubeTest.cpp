
#include "SimpleCubeTest.h"

#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"
#include "Camera/Camera.h"

#include <DXHelper.h>
#include <Timer/Timer.h>

#include <array>
#include <filesystem>
#include <iostream>
using namespace Microsoft::WRL;

using namespace DirectX;

OSimpleCubeTest::OSimpleCubeTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
    : OTest(_Engine, _Window)
{
}

bool OSimpleCubeTest::Initialize()
{
	SetupProjection();
	const auto engine = Engine.lock();
	assert(engine->GetCommandQueue()->GetCommandQueue());
	const auto queue = engine->GetCommandQueue();
	queue->ResetCommandList();

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	THROW_IF_FAILED(queue->GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { queue->GetCommandList().Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	engine->FlushGPU();
	ContentLoaded = true;

	return true;
}

void OSimpleCubeTest::UnloadContent()
{
	ContentLoaded = false;
}

void OSimpleCubeTest::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);

	// Convert Spherical to Cartesian coordinates.
	const float x = Radius * sinf(Phi) * cosf(Theta);
	const float z = Radius * sinf(Phi) * sinf(Theta);
	const float y = Radius * cosf(Phi);

	// Build the view matrix.
	const XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	const XMVECTOR target = XMVectorZero();
	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	const XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&ViewMatrix, view);

	const XMMATRIX world = XMLoadFloat4x4(&WorldMatrix);
	const XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	const XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	SObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(worldViewProj));
	ObjectCB->CopyData(0, objConstants);

	LOG(Test, Log, "Time: {}", Event.Timer.GetTime());
}

void OSimpleCubeTest::OnRender(const UpdateEventArgs& Event)
{
	auto engine = Engine.lock();
	auto commandList = engine->GetCommandQueue()->GetCommandList();
	auto window = Window.lock();
	auto allocator = engine->GetCommandQueue()->GetCommandAllocator();
	auto commandQueue = engine->GetCommandQueue();

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	THROW_IF_FAILED(allocator->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	THROW_IF_FAILED(commandList->Reset(allocator.Get(), PipelineStateObject.Get()));

	commandList->RSSetViewports(1, &window->Viewport);
	commandList->RSSetScissorRects(1, &window->ScissorRect);

	auto currentBackBuffer = window->GetCurrentBackBuffer().Get();
	auto dsv = window->GetDepthStensilView();
	auto rtv = window->CurrentBackBufferView();

	auto transitionPresentRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Indicate a state transition on the resource usage.
	commandList->ResourceBarrier(1, &transitionPresentRenderTarget);

	// Clear the back buffer and depth buffer.
	commandList->ClearRenderTargetView(rtv, Colors::LightSteelBlue, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

	ID3D12DescriptorHeap* descriptorHeaps[] = { CBVHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetGraphicsRootSignature(RootSignature.Get());

	const auto boxGeometryVertexBufferView = BoxGeometry->VertexBufferView();
	const auto boxGeometryIndexBufferView = BoxGeometry->IndexBufferView();

	commandList->IASetVertexBuffers(0, 1, &boxGeometryVertexBufferView);
	commandList->IASetIndexBuffer(&boxGeometryIndexBufferView);
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootDescriptorTable(0, CBVHeap->GetGPUDescriptorHandleForHeapStart());

	auto handleIncrementSize = Engine.lock()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_GPU_DESCRIPTOR_HANDLE cbvHeapHandleSecond = CBVHeap->GetGPUDescriptorHandleForHeapStart();
	cbvHeapHandleSecond.ptr += handleIncrementSize; // Move to the second descriptor

	// Set the second constant buffer (cbTimeObject at b1)
	commandList->SetGraphicsRootDescriptorTable(1, cbvHeapHandleSecond);

	commandList->DrawIndexedInstanced(
	    BoxGeometry->FindSubmeshGeomentry("Box").IndexCount,
	    1,
	    0,
	    0,
	    0);

	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	// Indicate a state transition on the resource usage.
	commandList->ResourceBarrier(1, &transition);

	// Done recording commands.
	THROW_IF_FAILED(commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	THROW_IF_FAILED(window->GetSwapChain()->Present(0, 0));
	window->MoveToNextFrame();
	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	engine->FlushGPU();
}

void OSimpleCubeTest::OnResize(const ResizeEventArgs& Event)
{
	OTest::OnResize(Event);
	SetupProjection();
}

void OSimpleCubeTest::SetupProjection()
{
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, Window.lock()->GetAspectRatio(), 1.0f, 1000.0f));
}

void OSimpleCubeTest::OnMouseWheel(const MouseWheelEventArgs& Event)
{
	const auto window = Window.lock();
	auto fov = window->GetFoV();
	fov -= Event.WheelDelta;
	fov = std::clamp(fov, 12.0f, 90.0f);
	window->SetFoV(fov);
	char buffer[256];
	sprintf_s(buffer, "FoV: %f\n", fov);
	OutputDebugStringA(buffer);
}

void OSimpleCubeTest::OnKeyPressed(const KeyEventArgs& Event)
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

void OSimpleCubeTest::OnMouseMoved(const MouseMotionEventArgs& Args)
{
	OTest::OnMouseMoved(Args);
	auto window = Window.lock();

	if (Args.LeftButton)
	{
		float dx = XMConvertToRadians(0.25f * (Args.X - window->GetLastXMousePos()));
		float dy = XMConvertToRadians(0.25f * (Args.Y - window->GetLastYMousePos()));

		Theta += dx;
		Phi += dy;

		Phi = std::clamp(Phi, 0.1f, XM_PI - 0.1f);
	}

	else if (Args.RightButton)
	{
		float dx = 0.005f * (Args.X - window->GetLastXMousePos());
		float dy = 0.005f * (Args.Y - window->GetLastYMousePos());
		Radius += dx - dy;

		Radius = std::clamp(Radius, 3.0f, 15.0f);
	}
	LOG(Test, Log, "Theta: {} Phi: {} Radius: {}", Theta, Phi, Radius);
}

void OSimpleCubeTest::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 2;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(Engine.lock()->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CBVHeap)));
}

void OSimpleCubeTest::BuildConstantBuffers()
{
	// Create the first constant buffer
	ObjectCB = make_unique<OUploadBuffer<SObjectConstants>>(Engine.lock()->GetDevice().Get(), 1, true);
	const auto objCBByteSize = Utils::CalcBufferByteSize(sizeof(SObjectConstants)); // Ensure this is 256-byte aligned

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ObjectCB->GetResource()->GetGPUVirtualAddress();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objCBByteSize;

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHeapHandle = CBVHeap->GetCPUDescriptorHandleForHeapStart();

	Engine.lock()->GetDevice()->CreateConstantBufferView(&cbvDesc, cbvHeapHandle);

	// Create the second constant buffer
}

void OSimpleCubeTest::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTimeTable;
	cbvTimeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTimeTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	auto hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
	}
	THROW_IF_FAILED(hr);
	THROW_IF_FAILED(Engine.lock()->GetDevice()->CreateRootSignature(0,
	                                                                serializedRootSig->GetBufferPointer(),
	                                                                serializedRootSig->GetBufferSize(),
	                                                                IID_PPV_ARGS(&RootSignature)));
}

void OSimpleCubeTest::BuildShadersAndInputLayout()
{
	MvsByteCode = Utils::CompileShader(L"Shaders/SimpleCubeShader.hlsl", nullptr, "VS", "vs_5_0");
	MpsByteCode = Utils::CompileShader(L"Shaders/SimpleCubeShader.hlsl", nullptr, "PS", "ps_5_0");

	InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void OSimpleCubeTest::BuildBoxGeometry()
{
	//TODO - FIX filling vertex buffer
	// const array vertices = {
	// 	SVertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
	// 	SVertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
	// 	SVertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
	// 	SVertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
	// 	SVertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
	// 	SVertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
	// 	SVertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
	// 	SVertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	// };
	//
	// //clang-format off
	// std::array<uint16_t, 36> indices = {
	// 	// front face
	// 	0,
	// 	1,
	// 	2,
	// 	0,
	// 	2,
	// 	3,
	//
	// 	// back face
	// 	4,
	// 	6,
	// 	5,
	// 	4,
	// 	7,
	// 	6,
	//
	// 	// left face
	// 	4,
	// 	5,
	// 	1,
	// 	4,
	// 	1,
	// 	0,
	//
	// 	// right face
	// 	3,
	// 	2,
	// 	6,
	// 	3,
	// 	6,
	// 	7,
	//
	// 	// top face
	// 	1,
	// 	5,
	// 	6,
	// 	1,
	// 	6,
	// 	2,
	//
	// 	// bottom face
	// 	4,
	// 	0,
	// 	3,
	// 	4,
	// 	3,
	// 	7
	// };
	// //clang-format on
	//
	// auto commandList = Engine.lock()->GetCommandQueue()->GetCommandList();
	// constexpr auto vbByteSize = vertices.size() * sizeof(SVertex);
	// constexpr auto ibByteSize = indices.size() * sizeof(uint16_t);
	//
	// BoxGeometry = make_unique<SMeshGeometry>();
	// BoxGeometry->Name = "BoxGeo";
	//
	// THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &BoxGeometry->VertexBufferCPU));
	// CopyMemory(BoxGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	//
	// THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &BoxGeometry->IndexBufferCPU));
	// CopyMemory(BoxGeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	//
	// BoxGeometry->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	//                                                           Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	//                                                           vertices.data(),
	//                                                           vbByteSize,
	//                                                           BoxGeometry->VertexBufferUploader);
	//
	// BoxGeometry->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	//                                                          Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	//                                                          indices.data(),
	//                                                          ibByteSize,
	//                                                          BoxGeometry->IndexBufferUploader);
	//
	// BoxGeometry->VertexByteStride = sizeof(SVertex);
	// BoxGeometry->VertexBufferByteSize = vbByteSize;
	// BoxGeometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	// BoxGeometry->IndexBufferByteSize = ibByteSize;
	//
	// SSubmeshGeometry submesh;
	// submesh.IndexCount = indices.size();
	// submesh.StartIndexLocation = 0;
	// submesh.BaseVertexLocation = 0;
	//
	// BoxGeometry->GetGeomentry("Box") = submesh;
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
	THROW_IF_FAILED(device->CreateCommittedResource(
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
		THROW_IF_FAILED(device->CreateCommittedResource(
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

void OSimpleCubeTest::BuildPSO()
{
	auto engine = Engine.lock();
	UINT quality = 0;
	bool msaaEnable = engine->GetMSAAState(quality);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
	psoDesc.pRootSignature = RootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(MvsByteCode->GetBufferPointer()), MvsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(MpsByteCode->GetBufferPointer()), MpsByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = SRenderConstants::BackBufferFormat;
	psoDesc.SampleDesc.Count = msaaEnable ? 4 : 1;
	psoDesc.SampleDesc.Quality = msaaEnable ? (quality - 1) : 0;
	psoDesc.DSVFormat = SRenderConstants::DepthBufferFormat;
	THROW_IF_FAILED(engine->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PipelineStateObject)));
}

#pragma optimize("", on)
