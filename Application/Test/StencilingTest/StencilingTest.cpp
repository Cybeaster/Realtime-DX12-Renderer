#include "StencilingTest.h"

#include "../../../Materials/Material.h"
#include "../../../Objects/GeomertryGenerator/GeometryGenerator.h"
#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"
#include "Camera/Camera.h"
#include "RenderConstants.h"
#include "RenderItem.h"
#include "Textures/DDSTextureLoader/DDSTextureLoader.h"

#include <DXHelper.h>
#include <Timer/Timer.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <ranges>

using namespace Microsoft::WRL;

using namespace DirectX;

OStencilingTest::OStencilingTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
    : OTest(_Engine, _Window)
{
}

bool OStencilingTest::Initialize()
{
	SetupProjection();

	const auto engine = Engine.lock();
	assert(engine->GetCommandQueue()->GetCommandQueue());
	const auto queue = engine->GetCommandQueue();
	queue->ResetCommandList();

	MainPassCB.FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	MainPassCB.FogStart = 5.0f;
	MainPassCB.FogRange = 150.0f;

	engine->CreateWaves(256, 256, 0.5f, 0.01f, 4.0f, 0.2f);
	CreateTexture();
	BuildRootSignature();
	BuildDescriptorHeap();
	BuildShadersAndInputLayout();
	BuildMaterials();
	BuildRenderItems();

	engine->BuildFrameResource();
	engine->BuildPSOs(RootSignature, InputLayout);
	THROW_IF_FAILED(queue->GetCommandList()->Close());

	ID3D12CommandList* cmdsLists[] = { queue->GetCommandList().Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	engine->FlushGPU();
	ContentLoaded = true;

	return true;
}

void OStencilingTest::UnloadContent()
{
	ContentLoaded = false;
}

void OStencilingTest::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);

	UpdateCamera();
	OnKeyboardInput(Event.Timer);

	auto engine = Engine.lock();
	engine->CurrentFrameResourceIndex = (engine->CurrentFrameResourceIndex + 1) % SRenderConstants::NumFrameResources;
	engine->CurrentFrameResources = engine->FrameResources[engine->CurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame
	// resource. If not, wait until the GPU has completed commands up to
	// this fence point.

	if (engine->CurrentFrameResources->Fence != 0 && engine->GetCommandQueue()->GetFence()->GetCompletedValue() < engine->CurrentFrameResources->Fence)
	{
		engine->GetCommandQueue()->WaitForFenceValue(engine->CurrentFrameResources->Fence);
	}

	AnimateMaterials(Event.Timer);
	UpdateObjectCBs(Event.Timer);
	UpdateMaterialCB();
	UpdateMainPass(Event.Timer);
}

void OStencilingTest::UpdateMainPass(const STimer& Timer)
{
	auto engine = Engine.lock();
	auto window = Window.lock();

	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	auto viewDet = XMMatrixDeterminant(view);
	auto projDet = XMMatrixDeterminant(proj);
	auto viewProjDet = XMMatrixDeterminant(viewProj);

	XMMATRIX invView = XMMatrixInverse(&viewDet, view);
	XMMATRIX invProj = XMMatrixInverse(&projDet, proj);
	XMMATRIX invViewProj = XMMatrixInverse(&viewProjDet, viewProj);

	XMStoreFloat4x4(&MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	MainPassCB.EyePosW = EyePos;
	MainPassCB.RenderTargetSize = XMFLOAT2((float)window->GetWidth(), (float)window->GetHeight());
	MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / window->GetWidth(), 1.0f / window->GetHeight());
	MainPassCB.NearZ = 1.0f;
	MainPassCB.FarZ = 1000.0f;
	MainPassCB.TotalTime = Timer.GetTime();
	MainPassCB.DeltaTime = Timer.GetDeltaTime();
	MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	MainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	MainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.9f };
	MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	MainPassCB.Lights[1].Strength = { 0.5f, 0.5f, 0.5f };
	MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = engine->CurrentFrameResources->PassCB.get();
	currPassCB->CopyData(0, MainPassCB);
}

void OStencilingTest::UpdateObjectCBs(const STimer& Timer)
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
			auto texTransform = XMLoadFloat4x4(&item->TexTransform);

			SObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			currentObjectCB->CopyData(item->ObjectCBIndex, objConstants);

			// Next FrameResource need to ber updated too
			item->NumFramesDirty--;
		}
	}
}

void OStencilingTest::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList, const vector<SRenderItem*>& RenderItems) const
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

		CD3DX12_GPU_DESCRIPTOR_HANDLE texDef(SRVHeap->GetGPUDescriptorHandleForHeapStart());
		texDef.Offset(renderItem->Material->DiffuseSRVHeapIndex, engine->CBVSRVUAVDescriptorSize);

		// Offset to the CBV in the descriptor heap for this object and
		// for this frame resource.

		const auto cbAddress = objectCB->GetGPUVirtualAddress() + renderItem->ObjectCBIndex * objectCBByteSize;
		const auto matCBAddress = materialCB->GetGPUVirtualAddress() + renderItem->Material->MaterialCBIndex * matCBByteSize;

		CommandList->SetGraphicsRootDescriptorTable(0, texDef);
		CommandList->SetGraphicsRootConstantBufferView(1, cbAddress);
		CommandList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		CommandList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
}

void OStencilingTest::UpdateCamera()
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

void OStencilingTest::OnKeyboardInput(const STimer& Timer)
{
	if (GetAsyncKeyState('1') & 0x8000)
	{
		MainPassCB.FogColor = { 0.7, 0.7, 0.7, 1.0 };
	}
	if (GetAsyncKeyState('2') & 0x8000)
	{
		MainPassCB.FogColor = { 1, 0, 0, 1.0 };
	}
	if (GetAsyncKeyState('3') & 0x8000)
	{
		MainPassCB.FogColor = { 0, 1, 0, 1.0 };
	}
	if (GetAsyncKeyState('4') & 0x8000)
	{
		MainPassCB.FogColor = { 0.7, 0.7, 0.7, 0.5 };
	}
}

void OStencilingTest::OnMouseWheel(const MouseWheelEventArgs& Args)
{
	OTest::OnMouseWheel(Args);
	MainPassCB.FogStart += Args.WheelDelta;
}

void OStencilingTest::CreateTexture()
{
	GetEngine()->CreateTexture(BricksTexture, L"Resources/Textures/bricks3.dds");
	GetEngine()->CreateTexture(CheckboardTexture, L"Resources/Textures/checkboard.dds");
	GetEngine()->CreateTexture(IceTexture, L"Resources/Textures/water1.dds");
	GetEngine()->CreateTexture(White1x1Texture, L"Resources/Textures/white1x1.dds");
}

void OStencilingTest::OnRender(const UpdateEventArgs& Event)
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
	switch (PSOMode)
	{
	case 0:
		THROW_IF_FAILED(commandList->Reset(allocator.Get(), GetEngine()->GetPSO(SPSOType::Opaque).Get()));
		break;
	case 1:
		THROW_IF_FAILED(commandList->Reset(allocator.Get(), GetEngine()->GetPSO(SPSOType::Debug).Get()));
		break;
	case 2:
		THROW_IF_FAILED(commandList->Reset(allocator.Get(), GetEngine()->GetPSO(SPSOType::Transparent).Get()));
		break;
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
	commandList->ClearRenderTargetView(rtv, reinterpret_cast<float*>(&MainPassCB.FogColor), 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

	ID3D12DescriptorHeap* heaps[] = { SRVHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	commandList->SetGraphicsRootSignature(RootSignature.Get());

	auto passCB = engine->CurrentFrameResources->PassCB->GetResource();
	commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	GetEngine()->SetPipelineState(SPSOType::Opaque);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Opaque));

	GetEngine()->SetPipelineState(SPSOType::AlphaTested);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::AlphaTested));

	GetEngine()->SetPipelineState(SPSOType::Transparent);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Transparent));

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

void OStencilingTest::OnResize(const ResizeEventArgs& Event)
{
	OTest::OnResize(Event);
	SetupProjection();
}

void OStencilingTest::SetupProjection()
{
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, Window.lock()->GetAspectRatio(), 1.0f, 1000.0f));
}

void OStencilingTest::BuildMaterials()
{
	auto grass = make_unique<SMaterial>();
	grass->Name = "Grass";
	grass->MaterialCBIndex = 0;
	grass->DiffuseSRVHeapIndex = 0;
	grass->MaterialConsatnts.DiffuseAlbedo = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	grass->MaterialConsatnts.FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->MaterialConsatnts.Roughness = 0.125f;

	auto water = make_unique<SMaterial>();
	water->Name = "Water";
	water->MaterialCBIndex = 1;
	water->DiffuseSRVHeapIndex = 1;
	water->MaterialConsatnts.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->MaterialConsatnts.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->MaterialConsatnts.Roughness = 0.0f;

	auto fence = make_unique<SMaterial>();
	fence->Name = "WireFence";
	fence->MaterialCBIndex = 2;
	fence->DiffuseSRVHeapIndex = 2;
	fence->MaterialConsatnts.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fence->MaterialConsatnts.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	fence->MaterialConsatnts.Roughness = 0.25f;

	auto fireball = make_unique<SMaterial>();
	fireball->Name = "FireBall";
	fireball->MaterialCBIndex = 3;
	fireball->DiffuseSRVHeapIndex = 3;
	fireball->MaterialConsatnts.DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	fireball->MaterialConsatnts.FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	fireball->MaterialConsatnts.Roughness = 0.25f;

	GetEngine()->AddMaterial(fence->Name, fence);
	GetEngine()->AddMaterial(grass->Name, grass);
	GetEngine()->AddMaterial(water->Name, water);
	GetEngine()->AddMaterial(fireball->Name, fireball);
}

void OStencilingTest::UpdateMaterialCB()
{
	const auto currentMaterialCB = Engine.lock()->CurrentFrameResources->MaterialCB.get();
	for (auto& materials = Engine.lock()->GetMaterials(); const auto& val : materials | std::views::values)
	{
		if (const auto material = val.get())
		{
			if (material->NumFramesDirty > 0)
			{
				const auto matTransform = XMLoadFloat4x4(&material->MaterialConsatnts.MatTransform);

				SMaterialConstants matConstants;
				matConstants.DiffuseAlbedo = material->MaterialConsatnts.DiffuseAlbedo;
				matConstants.FresnelR0 = material->MaterialConsatnts.FresnelR0;
				matConstants.Roughness = material->MaterialConsatnts.Roughness;
				XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

				currentMaterialCB->CopyData(material->MaterialCBIndex, matConstants);
				material->NumFramesDirty--;
			}
		}
	}
}

void OStencilingTest::OnKeyPressed(const KeyEventArgs& Event)
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

void OStencilingTest::OnMouseMoved(const MouseMotionEventArgs& Args)
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
		float dx = 0.05f * (Args.X - window->GetLastXMousePos());
		float dy = 0.05f * (Args.Y - window->GetLastYMousePos());
		Radius += dx - dy;

		Radius = std::clamp(Radius, 5.0f, 150.f);
	}
	LOG(Log, "Theta: {} Phi: {} Radius: {}", Theta, Phi, Radius);
}

void OStencilingTest::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	constexpr auto size = 4;
	CD3DX12_ROOT_PARAMETER slotRootParameter[size];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	const auto staticSamples = Utils::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(size, slotRootParameter, staticSamples.size(), staticSamples.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

void OStencilingTest::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO fogDefines[] = {
		"FOG", "1", NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] = {
		"FOG", "1", "ALPHA_TEST", "1", NULL, NULL
	};

	GetEngine()->BuildVSShader(L"Shaders/BaseShader.hlsl", SShaderTypes::VSBaseShader);
	GetEngine()->BuildPSShader(L"Shaders/BaseShader.hlsl", SShaderTypes::PSOpaque, fogDefines);
	GetEngine()->BuildPSShader(L"Shaders/BaseShader.hlsl", SShaderTypes::PSAlphaTested, alphaTestDefines);

	InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void OStencilingTest::BuildDescriptorHeap()
{
	auto device = Engine.lock()->GetDevice();
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&SRVHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(SRVHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTexture = GetEngine()->FindTexture(BricksTexture)->Resource;
	auto checkboardTexture = GetEngine()->FindTexture(CheckboardTexture)->Resource;
	auto whiteTexture = GetEngine()->FindTexture(White1x1Texture)->Resource;
	auto iceTexture = GetEngine()->FindTexture(IceTexture)->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTexture->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	device->CreateShaderResourceView(bricksTexture.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, GetEngine()->CBVSRVUAVDescriptorSize);

	srvDesc.Format = checkboardTexture->GetDesc().Format;
	device->CreateShaderResourceView(checkboardTexture.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, GetEngine()->CBVSRVUAVDescriptorSize);

	srvDesc.Format = whiteTexture->GetDesc().Format;
	device->CreateShaderResourceView(whiteTexture.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, GetEngine()->CBVSRVUAVDescriptorSize);

	srvDesc.Format = iceTexture->GetDesc().Format;
	device->CreateShaderResourceView(iceTexture.Get(), &srvDesc, hDescriptor);
}

void OStencilingTest::BuildRoomGeomety()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one SVertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/

	std::array<SVertex, 20> vertices = {
		// Floor: Observe we tile texture coordinates.
		SVertex(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0
		SVertex(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
		SVertex(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
		SVertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		SVertex(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
		SVertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
		SVertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

		SVertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8
		SVertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
		SVertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

		SVertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
		SVertex(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertex(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
		SVertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

		// Mirror
		SVertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 16
		SVertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		SVertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		SVertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)
	};

	std::array<std::int16_t, 30> indices = {
		// Floor
		0,
		1,
		2,
		0,
		2,
		3,

		// Walls
		4,
		5,
		6,
		4,
		6,
		7,

		8,
		9,
		10,
		8,
		10,
		11,

		12,
		13,
		14,
		12,
		14,
		15,

		// Mirror
		16,
		17,
		18,
		16,
		18,
		19
	};

	SSubmeshGeometry floorSubmesh;
	floorSubmesh.IndexCount = 6;
	floorSubmesh.StartIndexLocation = 0;
	floorSubmesh.BaseVertexLocation = 0;

	SSubmeshGeometry wallSubmesh;
	wallSubmesh.IndexCount = 18;
	wallSubmesh.StartIndexLocation = 6;
	wallSubmesh.BaseVertexLocation = 0;

	SSubmeshGeometry mirrorSubmesh;
	mirrorSubmesh.IndexCount = 6;
	mirrorSubmesh.StartIndexLocation = 24;
	mirrorSubmesh.BaseVertexLocation = 0;
	const UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(SVertex);
	const UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::uint16_t);

	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = "RoomGeometry";

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

	geo->SetGeometry("Floor", floorSubmesh);
	geo->SetGeometry("Wall", wallSubmesh);
	geo->SetGeometry("Mirror", mirrorSubmesh);

	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OStencilingTest::BuildRenderItems()
{
	auto wavesRenderItem = make_unique<SRenderItem>();
	wavesRenderItem->World = Utils::Math::Identity4x4();
	XMStoreFloat4x4(&wavesRenderItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRenderItem->ObjectCBIndex = 0;
	wavesRenderItem->Geometry = GetEngine()->GetSceneGeometry()["WaterGeometry"].get();
	wavesRenderItem->Material = GetEngine()->FindMaterial("Water");
	wavesRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRenderItem->IndexCount = wavesRenderItem->Geometry->FindSubmeshGeomentry("Grid").IndexCount;
	wavesRenderItem->StartIndexLocation = wavesRenderItem->Geometry->FindSubmeshGeomentry("Grid").StartIndexLocation;
	wavesRenderItem->BaseVertexLocation = wavesRenderItem->Geometry->FindSubmeshGeomentry("Grid").BaseVertexLocation;

	WavesRenderItem = wavesRenderItem.get();
	GetEngine()->AddRenderItem(SRenderLayer::Transparent, std::move(wavesRenderItem));

	auto gridRenderItem = make_unique<SRenderItem>();
	gridRenderItem->World = Utils::Math::Identity4x4();
	XMStoreFloat4x4(&gridRenderItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRenderItem->ObjectCBIndex = 1;
	gridRenderItem->Geometry = GetEngine()->GetSceneGeometry()["LandGeo"].get();
	gridRenderItem->Material = GetEngine()->FindMaterial("Grass");
	gridRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->IndexCount = gridRenderItem->Geometry->FindSubmeshGeomentry("Grid").IndexCount;
	gridRenderItem->StartIndexLocation = gridRenderItem->Geometry->FindSubmeshGeomentry("Grid").StartIndexLocation;
	gridRenderItem->BaseVertexLocation = gridRenderItem->Geometry->FindSubmeshGeomentry("Grid").BaseVertexLocation;
	XMStoreFloat4x4(&gridRenderItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));

	GetEngine()->AddRenderItem(SRenderLayer::Opaque, std::move(gridRenderItem));

	auto boxRenderItem = make_unique<SRenderItem>();
	XMStoreFloat4x4(&boxRenderItem->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	boxRenderItem->ObjectCBIndex = 2;
	boxRenderItem->Geometry = GetEngine()->GetSceneGeometry()["BoxGeometry"].get();
	boxRenderItem->Material = GetEngine()->FindMaterial("WireFence");
	boxRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRenderItem->IndexCount = boxRenderItem->Geometry->FindSubmeshGeomentry("Box").IndexCount;
	boxRenderItem->StartIndexLocation = boxRenderItem->Geometry->FindSubmeshGeomentry("Box").StartIndexLocation;
	boxRenderItem->BaseVertexLocation = boxRenderItem->Geometry->FindSubmeshGeomentry("Box").BaseVertexLocation;

	GetEngine()->AddRenderItem(SRenderLayer::AlphaTested, std::move(boxRenderItem));
}

float OStencilingTest::GetHillsHeight(float X, float Z) const
{
	return 0.3 * (Z * sinf(0.1f * X) + X * cosf(0.1f * Z));
}

void OStencilingTest::AnimateMaterials(const STimer& Timer)
{
	auto waterMaterial = GetEngine()->FindMaterial("FireBall");

	float& tu = waterMaterial->MaterialConsatnts.MatTransform(3, 0);
	float& tv = waterMaterial->MaterialConsatnts.MatTransform(3, 1);

	tu += 0.1f * Timer.GetDeltaTime();
	tv += 0.02f * Timer.GetDeltaTime();

	if (tu >= 1.0)
	{
		tu -= 1.0f;
	}

	if (tv >= 1.0)
	{
		tv -= 1.0f;
	}

	waterMaterial->MaterialConsatnts.MatTransform(3, 0) = tu;
	waterMaterial->MaterialConsatnts.MatTransform(3, 1) = tv;

	waterMaterial->NumFramesDirty = SRenderConstants::NumFrameResources;
}

XMFLOAT3 OStencilingTest::GetHillsNormal(float X, float Z) const
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
