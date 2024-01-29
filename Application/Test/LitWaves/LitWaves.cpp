
#include "LitWaves.h"

#include "../../../Materials/Material.h"
#include "../../../Objects/GeomertryGenerator/GeometryGenerator.h"
#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"
#include "Camera/Camera.h"
#include "RenderConstants.h"
#include "RenderItem.h"

#include <DXHelper.h>
#include <Timer/Timer.h>

#include <array>
#include <filesystem>
#include <iostream>

using namespace Microsoft::WRL;

using namespace DirectX;

OLitWaves::OLitWaves(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
    : OTest(_Engine, _Window)
{
}

bool OLitWaves::Initialize()
{
	SetupProjection();

	const auto engine = Engine.lock();
	assert(engine->GetCommandQueue()->GetCommandQueue());
	const auto queue = engine->GetCommandQueue();
	queue->ResetCommandList();


	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildMaterials();
	BuildRenderItems();
	BuildPSO();

	THROW_IF_FAILED(queue->GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { queue->GetCommandList().Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	engine->FlushGPU();
	ContentLoaded = true;

	return true;
}

void OLitWaves::UnloadContent()
{
	ContentLoaded = false;
}

void OLitWaves::UpdateWave(const STimer& Timer)
{

}

void OLitWaves::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);

	auto engine = Engine.lock();
	engine->CurrentFrameResourceIndex = (engine->CurrentFrameResourceIndex + 1) % SRenderConstants::NumFrameResources;
	engine->CurrentFrameResources = engine->FrameResources[engine->CurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame
	// resource. If not, wait until the GPU has completed commands up to
	// this fence point.

	UpdateCamera();
	OnKeyboardInput(Event.Timer);
	if (engine->CurrentFrameResources->Fence != 0 && engine->GetCommandQueue()->GetFence()->GetCompletedValue() < engine->CurrentFrameResources->Fence)
	{
		engine->GetCommandQueue()->WaitForFenceValue(engine->CurrentFrameResources->Fence);
	}
	UpdateObjectCBs(Event.Timer);
	UpdateMaterialCB();
	UpdateMainPass(Event.Timer);
	UpdateWave(Event.Timer);
}

void OLitWaves::UpdateMainPass(const STimer& Timer)
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
	MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	const XMVECTOR lightDir = -Utils::Math::SphericalToCartesian(1.0f, SunTheta, SunPhi);

	XMStoreFloat3(&MainPassCB.Lights[0].Direction, lightDir);
	MainPassCB.Lights[0].Strength = XMFLOAT3(sinf(Timer.GetTime() * 5), 1.0f, 1.0f);

	const auto currPassCB = engine->CurrentFrameResources->PassCB.get();
	currPassCB->CopyData(0, MainPassCB);
}

void OLitWaves::UpdateObjectCBs(const STimer& Timer)
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

void OLitWaves::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList, const vector<SRenderItem*>& RenderItems) const
{
	const auto engine = Engine.lock();

	auto matCBByteSize = Utils::CalcBufferByteSize(sizeof(SMaterialConstants));
	const auto objectCBByteSize = Utils::CalcBufferByteSize(sizeof(SObjectConstants));

	const auto objectCB = engine->CurrentFrameResources->ObjectCB->GetResource();
	const auto materialCB = engine->CurrentFrameResources->MaterialCB->GetResource();

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

		const auto cbAddress = objectCB->GetGPUVirtualAddress() + renderItem->ObjectCBIndex * objectCBByteSize;
		const auto matCBAddress = materialCB->GetGPUVirtualAddress() + renderItem->Material->MaterialCBIndex * matCBByteSize;

		CommandList->SetGraphicsRootConstantBufferView(0, cbAddress);
		CommandList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		CommandList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
}

void OLitWaves::UpdateCamera()
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

void OLitWaves::OnKeyboardInput(const STimer& Timer)
{
	const float deltaTime = Timer.GetDeltaTime();

	if (GetAsyncKeyState('1') & 0x8000)
	{
		bIsWireFrame = true;
	}
	else
	{
		bIsWireFrame = false;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		auto material = GetEngine()->FindMaterial("Water");
		material->MaterialConsatnts.Roughness = 1.f;
		material->NumFramesDirty = SRenderConstants::NumFrameResources;
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		auto material = GetEngine()->FindMaterial("Water");
		material->MaterialConsatnts.Roughness = 0.f;
		material->NumFramesDirty = SRenderConstants::NumFrameResources;
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		SunTheta -= 1.0f * deltaTime;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		SunTheta += 1.0f * deltaTime;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		SunPhi -= 1.0f * deltaTime;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		SunPhi += 1.0f * deltaTime;
	}
	SunPhi = Utils::Math::Clamp(SunPhi, 0.1f, XM_PIDIV2);
}

void OLitWaves::OnRender(const UpdateEventArgs& Event)
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
	commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Opaque));

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

void OLitWaves::OnResize(const ResizeEventArgs& Event)
{
	OTest::OnResize(Event);
	SetupProjection();
}

void OLitWaves::SetupProjection()
{
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, Window.lock()->GetAspectRatio(), 1.0f, 1000.0f));
}

void OLitWaves::BuildMaterials()
{
	auto grass = make_unique<SMaterial>();
	grass->Name = "Grass";
	grass->MaterialCBIndex = 0;
	grass->MaterialConsatnts.DiffuseAlbedo = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	grass->MaterialConsatnts.FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->MaterialConsatnts.Roughness = 1.f;

	auto water = make_unique<SMaterial>();
	water->Name = "Water";
	water->MaterialCBIndex = 1;
	water->MaterialConsatnts.DiffuseAlbedo = XMFLOAT4(0.0f, 0.2f, 0.6f, 1.0f);
	water->MaterialConsatnts.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->MaterialConsatnts.Roughness = 0.0f;

	GetEngine()->AddMaterial("Grass", grass);
	GetEngine()->AddMaterial("Water", water);
}

void OLitWaves::UpdateMaterialCB()
{
	const auto currentMaterialCB = Engine.lock()->CurrentFrameResources->MaterialCB.get();
	for (auto& materials = Engine.lock()->GetMaterials(); auto& elem : materials)
	{
		if (const auto material = elem.second.get())
		{
			if (material->NumFramesDirty > 0)
			{
				XMLoadFloat4x4(&material->MaterialConsatnts.MatTransform);

				SMaterialConstants matConstants;
				matConstants.DiffuseAlbedo = material->MaterialConsatnts.DiffuseAlbedo;
				matConstants.FresnelR0 = material->MaterialConsatnts.FresnelR0;
				matConstants.Roughness = material->MaterialConsatnts.Roughness;
				currentMaterialCB->CopyData(material->MaterialCBIndex, matConstants);
				material->NumFramesDirty--;
			}
		}
	}
}

void OLitWaves::OnKeyPressed(const KeyEventArgs& Event)
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

void OLitWaves::OnMouseMoved(const MouseMotionEventArgs& Args)
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

		Radius = std::clamp(Radius, 5.0f, 150.f);
	}
	LOG(Log, "Theta: {} Phi: {} Radius: {}", Theta, Phi, Radius);
}

void OLitWaves::BuildShadersAndInputLayout()
{
	GetEngine()->BuildShaders(L"Shaders/BaseShader.hlsl", "StandardVS", "OpaquePS");

	InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void OLitWaves::BuildLandGeometry()
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
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
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

	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OLitWaves::BuildWavesGeometryBuffers()
{
}

void OLitWaves::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
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

void OLitWaves::BuildPSO()
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

void OLitWaves::BuildRenderItems()
{
	auto wavesRenderItem = make_unique<SRenderItem>();
	wavesRenderItem->World = Utils::Math::Identity4x4();
	wavesRenderItem->ObjectCBIndex = 0;
	wavesRenderItem->Geometry = GetEngine()->GetSceneGeometry()["WaterGeometry"].get();
	wavesRenderItem->Material = GetEngine()->FindMaterial("Water");

	wavesRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRenderItem->IndexCount = wavesRenderItem->Geometry->FindSubmeshGeomentry("grid").IndexCount;
	wavesRenderItem->StartIndexLocation = wavesRenderItem->Geometry->FindSubmeshGeomentry("grid").StartIndexLocation;
	wavesRenderItem->BaseVertexLocation = wavesRenderItem->Geometry->FindSubmeshGeomentry("grid").BaseVertexLocation;
	wavesRenderItem->NumFramesDirty = SRenderConstants::NumFrameResources;

	WavesRenderItem = wavesRenderItem.get();
	GetEngine()->GetRenderItems(SRenderLayer::Opaque).push_back(wavesRenderItem.get());

	auto gridRenderItem = make_unique<SRenderItem>();
	gridRenderItem->World = Utils::Math::Identity4x4();
	gridRenderItem->ObjectCBIndex = 1;
	gridRenderItem->Geometry = GetEngine()->GetSceneGeometry()["LandGeo"].get();
	gridRenderItem->Material = GetEngine()->FindMaterial("Grass");
	gridRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->IndexCount = gridRenderItem->Geometry->FindSubmeshGeomentry("grid").IndexCount;
	gridRenderItem->StartIndexLocation = gridRenderItem->Geometry->FindSubmeshGeomentry("grid").StartIndexLocation;
	gridRenderItem->BaseVertexLocation = gridRenderItem->Geometry->FindSubmeshGeomentry("grid").BaseVertexLocation;

	GetEngine()->GetRenderItems(SRenderLayer::Opaque).push_back(gridRenderItem.get());
}

float OLitWaves::GetHillsHeight(float X, float Z) const
{
	return 0.3 * (Z * sinf(0.1f * X) + X * cosf(0.1f * Z));
}

XMFLOAT3 OLitWaves::GetHillsNormal(float X, float Z) const
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
