
#include "ShapesTest.h"

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


OShapesTest::OShapesTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
	: OTest(_Engine, _Window)
{
}

bool OShapesTest::Initialize()
{
	SetupProjection();

	const auto engine = Engine.lock();
	assert(engine->GetCommandQueue()->GetCommandQueue());
	const auto queue = engine->GetCommandQueue();
	queue->ResetCommandList();

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildRenderItems();
	engine->BuildFrameResource();

	BuildDescriptorHeaps();
	BuildConstantBuffersViews();
	BuildPSO();

	THROW_IF_FAILED(queue->GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { queue->GetCommandList().Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	engine->FlushGPU();
	ContentLoaded = true;

	return true;
}

void OShapesTest::UnloadContent()
{
	ContentLoaded = false;
}

void OShapesTest::OnUpdate(const UpdateEventArgs& Event)
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
}

void OShapesTest::UpdateMainPass(const STimer& Timer)
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

void OShapesTest::UpdateObjectCBs(const STimer& Timer)
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

void OShapesTest::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList, const vector<SRenderItem*>& RenderItems) const
{
	const auto engine = Engine.lock();
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

		const UINT cbvIndex = engine->CurrentFrameResourceIndex * engine->GetOpaqueRenderItems().size() + renderItem->ObjectCBIndex;
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CBVHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, engine->CBVSRVUAVDescriptorSize);

		CommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		CommandList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
}

void OShapesTest::UpdateCamera()
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

void OShapesTest::OnKeyboardInput()
{
	if (GetAsyncKeyState('1') & 0x8000)
		bIsWireFrame = true;
	else
		bIsWireFrame = false;
}

void OShapesTest::OnRender(const UpdateEventArgs& Event)
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

	ID3D12DescriptorHeap* descriptorHeaps[] = { CBVHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetGraphicsRootSignature(RootSignature.Get());

	auto passCBVIndex = PassConstantCBVOffset + engine->CurrentFrameResourceIndex;
	auto passCBVHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CBVHeap->GetGPUDescriptorHandleForHeapStart());
	passCBVHandle.Offset(passCBVIndex, engine->CBVSRVUAVDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(1, passCBVHandle);
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

void OShapesTest::OnResize(const ResizeEventArgs& Event)
{
	OTest::OnResize(Event);
	SetupProjection();
}

void OShapesTest::SetupProjection()
{
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, Window.lock()->GetAspectRatio(), 1.0f, 1000.0f));
}

void OShapesTest::OnMouseWheel(const MouseWheelEventArgs& Event)
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

void OShapesTest::OnKeyPressed(const KeyEventArgs& Event)
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

void OShapesTest::OnMouseMoved(const MouseMotionEventArgs& Args)
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
	LOG(Log, "Theta: {} Phi: {} Radius: {}", Theta, Phi, Radius);
}

void OShapesTest::BuildDescriptorHeaps()
{
	auto engine = Engine.lock();
	const UINT objectCount = engine->GetOpaqueRenderItems().size();
	const UINT numDescriptors = (objectCount + 1) * SRenderConstants::NumFrameResources;

	PassConstantCBVOffset = objectCount * SRenderConstants::NumFrameResources;
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(Engine.lock()->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CBVHeap)));
}

void OShapesTest::BuildConstantBuffersViews()
{
	const auto engine = Engine.lock();
	const UINT objectCBByteSize = Utils::CalcBufferByteSize(sizeof(SObjectConstants));
	const UINT objectCount = engine->GetOpaqueRenderItems().size();

	auto cmdList = engine->GetCommandQueue()->GetCommandList();
	for (uint32_t frameIndex = 0; frameIndex < SRenderConstants::NumFrameResources; ++frameIndex)
	{
		const auto objectCB = engine->FrameResources[frameIndex]->ObjectCB->GetResource();
		for (UINT i = 0; i < objectCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// Offset to the ith object constant buffer in the buffer.
			cbAddress += i * objectCBByteSize;

			//offset to the object cbv in the descriptor heap
			const int heapIndex = frameIndex * objectCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CBVHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, engine->CBVSRVUAVDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objectCBByteSize;
			engine->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	const UINT passCBByteSize = Utils::CalcBufferByteSize(sizeof(SPassConstants));
	for (UINT frameIndex = 0; frameIndex < SRenderConstants::NumFrameResources; ++frameIndex)
	{
		const auto passCB = engine->FrameResources[frameIndex]->PassCB->GetResource();
		const D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap
		const int heapIndex = PassConstantCBVOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CBVHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, engine->CBVSRVUAVDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;
		engine->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void OShapesTest::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

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

void OShapesTest::BuildShadersAndInputLayout()
{
	GetEngine()->BuildShaders(L"Shaders/BaseShader.hlsl", "StandardVS", "OpaquePS");

	InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void OShapesTest::BuildShapeGeometry()
{
	OGeometryGenerator generator;
	auto box = generator.CreateBox(1.5f, 0.5f, 1.5f, 3);
	auto grid = generator.CreateGrid(20.0f, 30.0f, 60, 40);
	auto sphere = generator.CreateSphere(0.5f, 20, 20);
	auto cylinder = generator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index
	// buffer. So define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex
	// buffer.

	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = static_cast<UINT>(box.Vertices.size());
	UINT sphereVertexOffset = gridVertexOffset + static_cast<UINT>(grid.Vertices.size());
	UINT cylinderVertexOffset = sphereVertexOffset + static_cast<UINT>(sphere.Vertices.size());

	// Cache the starting index for each object in the concatenated index
	// buffer.

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = static_cast<UINT>(box.Indices32.size());
	UINT sphereIndexOffset = gridIndexOffset + static_cast<UINT>(grid.Indices32.size());
	UINT cylinderIndexOffset = sphereIndexOffset + static_cast<UINT>(sphere.Indices32.size());

	// Define the SubmeshGeometry that cover different
	// regions of the vertex/index buffers.

	SSubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = static_cast<UINT>(box.Indices32.size());
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SSubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = static_cast<UINT>(grid.Indices32.size());
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SSubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = static_cast<UINT>(sphere.Indices32.size());
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SSubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = static_cast<UINT>(cylinder.Indices32.size());
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount = box.Vertices.size() + grid.Vertices.size() + sphere.Vertices.size() + cylinder.Vertices.size();

	vector<SVertex> vertices(totalVertexCount);

	vector<uint16_t> indices;
	indices.insert(indices.end(), begin(box.GetIndices16()), end(box.GetIndices16()));
	indices.insert(indices.end(), begin(grid.GetIndices16()), end(grid.GetIndices16()));
	indices.insert(indices.end(), begin(sphere.GetIndices16()), end(sphere.GetIndices16()));
	indices.insert(indices.end(), begin(cylinder.GetIndices16()), end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = make_unique<SMeshGeometry>();
	geo->Name = "shapeGeo";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	auto engine = Engine.lock();
	auto cmList = engine->GetCommandQueue()->GetCommandList();

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                  cmList.Get(),
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                 cmList.Get(),
	                                                 indices.data(),
	                                                 ibByteSize,
	                                                 geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->SetGeometry("Box", boxSubmesh);
	geo->SetGeometry("Grid", gridSubmesh);
	geo->SetGeometry("Sphere", sphereSubmesh);
	geo->SetGeometry("Cylinder", cylinderSubmesh);

	GetEngine()->GetSceneGeometry()[geo->Name] = std::move(geo);
}

void OShapesTest::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
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

void OShapesTest::BuildPSO()
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

void OShapesTest::BuildRenderItems()
{
	auto boxRenderItem = make_unique<SRenderItem>();
	const auto engine = Engine.lock();

	XMStoreFloat4x4(&boxRenderItem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));

	boxRenderItem->ObjectCBIndex = 0;
	boxRenderItem->Geometry = GetEngine()->GetSceneGeometry()["shapeGeo"].get();
	boxRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRenderItem->IndexCount = boxRenderItem->Geometry->GetGeomentry("Box").IndexCount;
	boxRenderItem->StartIndexLocation = boxRenderItem->Geometry->GetGeomentry("Box").StartIndexLocation;
	boxRenderItem->BaseVertexLocation = boxRenderItem->Geometry->GetGeomentry("Box").BaseVertexLocation;


	auto gridRenderItem = make_unique<SRenderItem>();
	gridRenderItem->World = Utils::Math::Identity4x4();
	gridRenderItem->ObjectCBIndex = 1;
	gridRenderItem->Geometry = GetEngine()->GetSceneGeometry()["shapeGeo"].get();
	gridRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->IndexCount = gridRenderItem->Geometry->GetGeomentry("Grid").IndexCount;
	gridRenderItem->StartIndexLocation = gridRenderItem->Geometry->GetGeomentry("Grid").StartIndexLocation;
	gridRenderItem->BaseVertexLocation = gridRenderItem->Geometry->GetGeomentry("Grid").BaseVertexLocation;

	// Build the columns and spheres in rows as in Figure 7.6.
	UINT objCBIndex = 2;
	for (uint32_t i = 0; i < 5; ++i)
	{
		auto leftCylinderRenderItem = make_unique<SRenderItem>();
		auto rightCylinderRenderItem = make_unique<SRenderItem>();
		auto leftSphereRenderItem = make_unique<SRenderItem>();
		auto rightSphereRenderItem = make_unique<SRenderItem>();

		const XMMATRIX leftCylinderWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		const XMMATRIX rightCylinderWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);
		const XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
		const XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);

		XMStoreFloat4x4(&leftCylinderRenderItem->World, rightCylinderWorld);
		leftCylinderRenderItem->ObjectCBIndex = objCBIndex++;
		leftCylinderRenderItem->Geometry = GetEngine()->GetSceneGeometry()["shapeGeo"].get();
		leftCylinderRenderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylinderRenderItem->IndexCount = leftCylinderRenderItem->Geometry->GetGeomentry("Cylinder").IndexCount;
		leftCylinderRenderItem->StartIndexLocation = leftCylinderRenderItem->Geometry->GetGeomentry("Cylinder").StartIndexLocation;
		leftCylinderRenderItem->BaseVertexLocation = leftCylinderRenderItem->Geometry->GetGeomentry("Cylinder").BaseVertexLocation;

		XMStoreFloat4x4(&rightCylinderRenderItem->World, leftCylinderWorld);
		rightCylinderRenderItem->ObjectCBIndex = objCBIndex++;
		rightCylinderRenderItem->Geometry = GetEngine()->GetSceneGeometry()["shapeGeo"].get();
		rightCylinderRenderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylinderRenderItem->IndexCount = rightCylinderRenderItem->Geometry->GetGeomentry("Cylinder").IndexCount;
		rightCylinderRenderItem->StartIndexLocation = rightCylinderRenderItem->Geometry->GetGeomentry("Cylinder").StartIndexLocation;
		rightCylinderRenderItem->BaseVertexLocation = rightCylinderRenderItem->Geometry->GetGeomentry("Cylinder").BaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRenderItem->World, leftSphereWorld);
		leftSphereRenderItem->ObjectCBIndex = objCBIndex++;
		leftSphereRenderItem->Geometry = GetEngine()->GetSceneGeometry()["shapeGeo"].get();
		leftSphereRenderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRenderItem->IndexCount = leftSphereRenderItem->Geometry->GetGeomentry("Sphere").IndexCount;
		leftSphereRenderItem->StartIndexLocation = leftSphereRenderItem->Geometry->GetGeomentry("Sphere").StartIndexLocation;
		leftSphereRenderItem->BaseVertexLocation = leftSphereRenderItem->Geometry->GetGeomentry("Sphere").BaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRenderItem->World, rightSphereWorld);
		rightSphereRenderItem->ObjectCBIndex = objCBIndex++;
		rightSphereRenderItem->Geometry = GetEngine()->GetSceneGeometry()["shapeGeo"].get();
		rightSphereRenderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRenderItem->IndexCount = rightSphereRenderItem->Geometry->GetGeomentry("Sphere").IndexCount;
		rightSphereRenderItem->StartIndexLocation = rightSphereRenderItem->Geometry->GetGeomentry("Sphere").StartIndexLocation;
		rightSphereRenderItem->BaseVertexLocation = rightSphereRenderItem->Geometry->GetGeomentry("Sphere").BaseVertexLocation;


	}

}

#pragma optimize("", on)

