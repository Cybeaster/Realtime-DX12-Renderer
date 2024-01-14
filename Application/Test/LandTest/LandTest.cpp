
#include "LandTest.h"

#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"
#include "Camera/Camera.h"
#include "RenderConstants.h"
#include "RenderItem.h"
#include "../../../Objects/GeomertryGenerator/GeometryGenerator.h"

#include <DXHelper.h>
#include <Timer/Timer.h>

#include <array>
#include <filesystem>
#include <iostream>
using namespace Microsoft::WRL;

using namespace DirectX;

OLandTest::OLandTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
	: OTest(_Engine, _Window)
{
}

bool OLandTest::Initialize()
{
	SetupProjection();

	const auto engine = Engine.lock();
	assert(engine->GetCommandQueue()->GetCommandQueue());
	const auto queue = engine->GetCommandQueue();
	queue->ResetCommandList();

	engine->CreateWaves(256, 256, 0.5f, 0.01f, 4.0f, 0.2f);

	WavesVB = make_unique<OUploadBuffer<SVertex>>(engine->GetDevice().Get(), 256, false);

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildRenderItems();
	engine->BuildFrameResource();
	BuildPSO();

	THROW_IF_FAILED(queue->GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { queue->GetCommandList().Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	engine->FlushGPU();
	ContentLoaded = true;

	return true;
}

void OLandTest::UnloadContent()
{
	ContentLoaded = false;
}

void OLandTest::UpdateWave(const STimer& Timer)
{
	static float tBase = 0.0f;
	if (Timer.GetTime() - tBase >= 0.25)
	{
		tBase += 0.25;
		int i = Utils::Math::Random(4, GetEngine()->GetWaves()->GetRowCount() - 5);
		int j = Utils::Math::Random(4, GetEngine()->GetWaves()->GetColumnCount() - 5);
		float r = Utils::Math::Random(0.2f, 0.5f);
		GetEngine()->GetWaves()->Disturb(i, j, r);
	}

	GetEngine()->GetWaves()->Update(Timer.GetDeltaTime());
	auto currWavesVB = Engine.lock()->CurrentFrameResources->WavesVB.get();
	for (int32_t i = 0; i < GetEngine()->GetWaves()->GetVertexCount(); ++i)
	{
		SVertex v;
		v.Pos = GetEngine()->GetWaves()->GetPosition(i);
		//v.Color = XMFLOAT4(Colors::Blue); /TODO fix color filling
		currWavesVB->CopyData(i, v);
	}
	WavesRenderItem->Geometry->VertexBufferGPU = currWavesVB->GetResource();
}

void OLandTest::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);

	auto engine = Engine.lock();
	engine->CurrentFrameResourceIndex = (engine->CurrentFrameResourceIndex + 1) % SRenderConstants::NumFrameResources;
	engine->CurrentFrameResources = engine->FrameResources[engine->CurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame
	// resource. If not, wait until the GPU has completed commands up to
	// this fence point.

	UpdateCamera();
	OnKeyboardInput();
	if (engine->CurrentFrameResources->Fence != 0 && engine->GetCommandQueue()->GetFence()->GetCompletedValue() < engine->CurrentFrameResources->Fence)
	{
		engine->GetCommandQueue()->WaitForFenceValue(engine->CurrentFrameResources->Fence);
	}

	UpdateMainPass(Event.Timer);
	UpdateObjectCBs(Event.Timer);
	UpdateWave(Event.Timer);
}

void OLandTest::UpdateMainPass(const STimer& Timer)
{
	auto engine = Engine.lock();
	auto window = Window.lock();

	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	auto detView = XMMatrixDeterminant(view);
	auto detProj = XMMatrixDeterminant(proj);
	auto detViewProj = XMMatrixDeterminant(viewProj);

	const XMMATRIX invView = XMMatrixInverse(&detView, view);
	const XMMATRIX invProj = XMMatrixInverse(&detProj, proj);
	const XMMATRIX invViewProj = XMMatrixInverse(&detViewProj, viewProj);

	XMStoreFloat4x4(&MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	MainPassCB.EyePosW = EyePos;
	MainPassCB.RenderTargetSize = XMFLOAT2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
	MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / window->GetWidth(), 1.0f / window->GetHeight());
	MainPassCB.NearZ = 1.0f;
	MainPassCB.FarZ = 1000.0f;
	MainPassCB.TotalTime = Timer.GetTime();
	MainPassCB.DeltaTime = Timer.GetDeltaTime();

	const auto currPassCB = engine->CurrentFrameResources->PassCB.get();
	currPassCB->CopyData(0, MainPassCB);
}

void OLandTest::UpdateObjectCBs(const STimer& Timer)
{
	const auto engine = Engine.lock();
	const auto currentObjectCB = engine->CurrentFrameResources->ObjectCB.get();

	for (const auto& item : engine->GetAllRenderItems())
	{
		// Only update the cbuffer data if the constants have changed.
		// This needs to be tracked per frame resource.

		if (item->NumFramesDirty > 0)
		{
			auto world = XMLoadFloat4x4(&item->World);
			SObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			currentObjectCB->CopyData(item->ObjectCBIndex, objConstants);

			// Next FrameResource need to ber updated too
			item->NumFramesDirty--;
		}
	}
}

void OLandTest::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList, const vector<SRenderItem*>& RenderItems) const
{
	const auto engine = Engine.lock();

	auto objectCBByteSize = Utils::CalcBufferByteSize(sizeof(SObjectConstants));
	auto objectCB = engine->CurrentFrameResources->ObjectCB->GetResource();

	for (size_t i = 0; i < RenderItems.size(); i++)
	{
		const auto renderItem = RenderItems[i];

		auto vertexView = renderItem->Geometry->VertexBufferView();
		auto indexView = renderItem->Geometry->IndexBufferView();

		CommandList->IASetVertexBuffers(0, 1, &vertexView);
		CommandList->IASetIndexBuffer(&indexView);
		CommandList->IASetPrimitiveTopology(renderItem->PrimitiveType);

		// Offset to the CBV in the descriptor heap for this object and
		// for this frame resource.

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();
		cbAddress += renderItem->ObjectCBIndex * objectCBByteSize;

		CommandList->SetGraphicsRootConstantBufferView(0, cbAddress);
		CommandList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
}

void OLandTest::UpdateCamera()
{
	// Convert Spherical to Cartesian coordinates.
	EyePos.x = Radius * sinf(Phi) * cosf(Theta);
	EyePos.z = Radius * sinf(Phi) * sinf(Theta);
	EyePos.y = Radius * cosf(Phi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(EyePos.x, EyePos.y, EyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&ViewMatrix, view);
}

void OLandTest::OnKeyboardInput()
{
	if (GetAsyncKeyState('1') & 0x8000)
		bIsWireFrame = true;
	else
		bIsWireFrame = false;
}

void OLandTest::OnRender(const UpdateEventArgs& Event)
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

	if (bIsWireFrame)
	{
		THROW_IF_FAILED(commandList->Reset(allocator.Get(), GetEngine()->GetPSO("OpaqueWireframe").Get()));
	}
	else
	{
		THROW_IF_FAILED(commandList->Reset(allocator.Get(), GetEngine()->GetPSO("Opaque").Get()));
	}

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

	commandList->SetGraphicsRootSignature(RootSignature.Get());

	auto passCB = engine->CurrentFrameResources->PassCB->GetResource();
	commandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	DrawRenderItems(commandList.Get(), engine->GetOpaqueRenderItems());

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

	// Add an instruction to the command queue to set a new fence point.
	// Because we are on the GPU timeline, the new fence point won’t be
	// set until the GPU finishes processing all the commands prior to
	// this Signal().
	engine->CurrentFrameResources->Fence = commandQueue->Signal();

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	engine->FlushGPU();
}

void OLandTest::OnResize(const ResizeEventArgs& Event)
{
	OTest::OnResize(Event);
	SetupProjection();
}

void OLandTest::SetupProjection()
{
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, Window.lock()->GetAspectRatio(), 1.0f, 1000.0f));
}

void OLandTest::OnKeyPressed(const KeyEventArgs& Event)
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

void OLandTest::OnMouseMoved(const MouseMotionEventArgs& Args)
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

		Radius = std::clamp(Radius, 3.0f, 35.f);
	}
	LOG(Log, "Theta: {} Phi: {} Radius: {}", Theta, Phi, Radius);
}

void OLandTest::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);

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

void OLandTest::BuildShadersAndInputLayout()
{
	GetEngine()->BuildShaders(L"Shaders/BaseShader.hlsl", "StandardVS", "OpaquePS");

	InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void OLandTest::BuildLandGeometry()
{
	OGeometryGenerator generator;
	auto grid = generator.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// Extract the vertex elements we are interested and apply the height
	// function to each vertex. In addition, color the vertices based on
	// their height so we have sandy looking beaches, grassy low hills,
	// and snow mountain peaks.
	//
	std::vector<SVertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		// Color the vertex based on its height.
		// if (vertices[i].Pos.y < -10.0f)
		//TODO Fix color filling
		// {
		// 	// Sandy beach color.
		// 	vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		// }
		// else if (vertices[i].Pos.y < 5.0f)
		// {
		// 	// Light yellow-green.
		// 	vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		// }
		// else if (vertices[i].Pos.y < 12.0f)
		// {
		// 	// Dark yellow-green.
		// 	vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		// }
		// else if (vertices[i].Pos.y < 20.0f)
		// {
		// 	// Dark brown.
		// 	vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		// }
		// else
		// {
		// 	// White snow.
		// 	vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		// }
	}

	const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(SVertex));
	const std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(std::uint16_t));

	auto geo = make_unique<SMeshGeometry>();
	geo->Name = "landGeo";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                  Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                 Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	                                                 indices.data(),
	                                                 ibByteSize,
	                                                 geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry geometry;
	geometry.IndexCount = static_cast<UINT>(indices.size());
	geometry.StartIndexLocation = 0;
	geometry.BaseVertexLocation = 0;
	geo->SetGeometry("grid", geometry);

	GetEngine()->SetSceneGeometry("LandGeo", std::move(geo));
}

void OLandTest::BuildWavesGeometryBuffers()
{
	vector<uint16_t> indices(3 * GetEngine()->GetWaves()->GetTriangleCount());

	//iterate over each quad
	int m = GetEngine()->GetWaves()->GetRowCount();
	int n = GetEngine()->GetWaves()->GetColumnCount();
	int k = 0;

	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = GetEngine()->GetWaves()->GetVertexCount() * sizeof(SVertex);
	UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(uint16_t));

	auto geometry = make_unique<SMeshGeometry>();
	geometry->Name = "WaterGeometry";
	geometry->VertexBufferCPU = nullptr;
	geometry->VertexBufferGPU = nullptr;

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geometry->IndexBufferCPU));
	CopyMemory(geometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                      Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	                                                      indices.data(),
	                                                      ibByteSize,
	                                                      geometry->IndexBufferUploader);

	geometry->VertexByteStride = sizeof(SVertex);
	geometry->VertexBufferByteSize = vbByteSize;
	geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	geometry->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geometry->SetGeometry("grid", submesh);
	GetEngine()->SetSceneGeometry("WaterGeometry", std::move(geometry));
}

void OLandTest::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
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

void OLandTest::BuildPSO()
{
	auto engine = Engine.lock();
	UINT quality = 0;
	bool msaaEnable = engine->GetMSAAState(quality);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
	psoDesc.pRootSignature = RootSignature.Get();

	auto vsShader = GetEngine()->GetShader("StandardVS");
	auto psShader = GetEngine()->GetShader("OpaquePS");

	psoDesc.VS = { reinterpret_cast<BYTE*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };

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
	engine->CreatePSO("Opaque", psoDesc);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = psoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	engine->CreatePSO("OpaqueWireframe", opaqueWireframePsoDesc);
}

void OLandTest::BuildRenderItems()
{
	auto wavesRenderItem = make_unique<SRenderItem>();
	wavesRenderItem->World = Utils::Math::Identity4x4();
	wavesRenderItem->ObjectCBIndex = 0;
	wavesRenderItem->Geometry = GetEngine()->GetSceneGeometry()["WaterGeometry"].get();
	wavesRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRenderItem->IndexCount = wavesRenderItem->Geometry->GetGeomentry("grid").IndexCount;
	wavesRenderItem->StartIndexLocation = wavesRenderItem->Geometry->GetGeomentry("grid").StartIndexLocation;
	wavesRenderItem->BaseVertexLocation = wavesRenderItem->Geometry->GetGeomentry("grid").BaseVertexLocation;
	wavesRenderItem->NumFramesDirty = SRenderConstants::NumFrameResources;

	WavesRenderItem = wavesRenderItem.get();
	GetEngine()->GetOpaqueRenderItems().push_back(wavesRenderItem.get());

	auto gridRenderItem = make_unique<SRenderItem>();
	gridRenderItem->World = Utils::Math::Identity4x4();
	gridRenderItem->ObjectCBIndex = 1;
	gridRenderItem->Geometry = GetEngine()->GetSceneGeometry()["LandGeo"].get();
	gridRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->IndexCount = gridRenderItem->Geometry->GetGeomentry("grid").IndexCount;
	gridRenderItem->StartIndexLocation = gridRenderItem->Geometry->GetGeomentry("grid").StartIndexLocation;
	gridRenderItem->BaseVertexLocation = gridRenderItem->Geometry->GetGeomentry("grid").BaseVertexLocation;

	GetEngine()->GetOpaqueRenderItems().push_back(gridRenderItem.get());
}

float OLandTest::GetHillsHeight(float X, float Z) const
{
	return 0.3 * (Z * sinf(0.1f * X) + X * cosf(0.1f * Z));
}

XMFLOAT3 OLandTest::GetHillsNormal(float X, float Z) const
{
	XMFLOAT3 n(
		-0.03f * Z * cosf(0.1f * X) - 0.3f * cosf(0.1f * Z),
		1.0f,
		-0.3f * sinf(0.1f * X) + 0.03f * X * sinf(0.1f * Z));

	const XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);
	return n;
}

#pragma optimize("", on)

