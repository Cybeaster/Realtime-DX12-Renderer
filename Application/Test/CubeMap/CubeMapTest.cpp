//
// Created by Cybea on 21/02/2024.
//

#include "CubeMapTest.h"

#include "../../../Utils/EngineHelper.h"
bool OCubeMapTest::Initialize()
{
	OEngine::Get()->BuildCubeRenderTarget({ 0, 2, 0 });
	BuildRenderItems();
	return true;
}

void OCubeMapTest::OnUpdate(const UpdateEventArgs& Event)
{
	OTest::OnUpdate(Event);
}

void OCubeMapTest::OnRender(const UpdateEventArgs& Event)
{
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList();
	DrawSceneToCubeMap();

	cmdList->RSSetViewports(1, &OEngine::Get()->GetWindow()->Viewport);
	cmdList->RSSetScissorRects(1, &OEngine::Get()->GetWindow()->ScissorRect);

	auto window = OEngine::Get()->GetWindow();

	auto backBuffer = window->GetCurrentBackBuffer();
	auto depthStencil = window->GetCurrentDepthStencilBuffer();

	auto backbufferView = window->CurrentBackBufferView();
	auto depthStencilView = window->GetDepthStensilView();

	Utils::ResourceBarrier(cmdList.Get(), window->GetCurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear the back buffer and depth buffer.
	cmdList->ClearRenderTargetView(backbufferView, DirectX::Colors::LightSteelBlue, 0, nullptr);
	cmdList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &backbufferView, true, &depthStencilView);

	auto passCB = OEngine::Get()->CurrentFrameResources->PassCB->GetResource();
	cmdList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	auto cubeMap = OEngine::Get()->GetCubeRenderTarget();
	cmdList->SetGraphicsRootDescriptorTable(5, cubeMap->GetSRVHandle().GPUHandle);
	OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::OpaqueDynamicReflections);
	cmdList->SetGraphicsRootDescriptorTable(5, OEngine::Get()->GetSRVDescHandleForTexture(FindTextureByName("grasscube1024.dds")));
	OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::Opaque);
	OEngine::Get()->DrawRenderItems(SPSOType::Sky, SRenderLayer::Sky);
}

void OCubeMapTest::BuildRenderItems()
{
	auto sky = CreateSphereRenderItem(SRenderLayer::Sky,
	                                  "Sky",
	                                  0.5f,
	                                  20,
	                                  20,
	                                  FindMaterial("GrassSky"));
	XMStoreFloat4x4(&sky[0].World, DirectX::XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));

	CreateRenderItem(SRenderLayer::Opaque,
	                 "Skull",
	                 "Resources/Models/skull.txt",
	                 EParserType::Custom,
	                 ETextureMapType::Spherical,
	                 SRenderItemParams{ FindMaterial("White") });

	auto box = CreateBoxRenderItem(SRenderLayer::Opaque,
	                               "Box",
	                               1.0f,
	                               1.0f,
	                               1.0f,
	                               3,
	                               FindMaterial("Tile"));
	box[0].World = Scale(Translate(box[0].World, { 2.0f, 1.0f, 2.0f }), { 1.0f, 2.0f, 1.0f });

	auto globe = CreateSphereRenderItem(SRenderLayer::OpaqueDynamicReflections,
	                                    "Globe",
	                                    0.5f,
	                                    20,
	                                    20,
	                                    FindMaterial("Mirror"));
	globe[0].World = Translate(Scale(globe[0].World, { 2.0f, 2.0f, 2.0f }), { 0, 2, 0 });

	auto grid = CreateGridRenderItem(SRenderLayer::Opaque,
	                                 "Grid",
	                                 20.0f,
	                                 30.0f,
	                                 60,
	                                 40,
	                                 FindMaterial("Tile"));
	Scale(grid[0].TexTransform, { 8, 8, 1.0f });

	auto cylinder = CreateCylinderRenderItem(SRenderLayer::Opaque,
	                                         "Cylinder",
	                                         0.5f,
	                                         0.5f,
	                                         3.0f,
	                                         20,
	                                         20,
	                                         { FindMaterial("Bricks"), 10 });

	auto spheres = CreateSphereRenderItem(SRenderLayer::Opaque,
	                                      "Spheres",
	                                      0.5f,
	                                      20,
	                                      20,
	                                      { FindMaterial("Mirror"), 10 });

	using namespace DirectX;
	XMMATRIX brickTexTransform = XMMatrixScaling(1.5f, 2.0f, 1.0f);
	for (size_t it = 0; it < 10; it += 2)
	{
		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + it * 5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + it * 5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + it * 5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + it * 5.0f);

		auto leftSphere = spheres[it];
		auto rightSphere = spheres[it + 1];

		auto leftCylinder = cylinder[it];
		auto rightCylinder = cylinder[it + 1];

		XMStoreFloat4x4(&leftSphere.World, leftSphereWorld);
		XMStoreFloat4x4(&rightSphere.World, rightSphereWorld);
		XMStoreFloat4x4(&rightCylinder.TexTransform, brickTexTransform);
		XMStoreFloat4x4(&leftCylinder.TexTransform, brickTexTransform);
		XMStoreFloat4x4(&leftCylinder.World, leftCylWorld);
		XMStoreFloat4x4(&rightCylinder.World, rightCylWorld);
	}
}

void OCubeMapTest::DrawSceneToCubeMap()
{
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList();
	auto cubeMap = OEngine::Get()->GetCubeRenderTarget();

	cmdList->SetGraphicsRootDescriptorTable(5, OEngine::Get()->GetSRVDescHandleForTexture(FindTextureByName("grasscube1024")));
	cmdList->RSSetViewports(1, &cubeMap->GetViewport());
	cmdList->RSSetScissorRects(1, &cubeMap->GetScissorRect());
	Utils::ResourceBarrier(cmdList.Get(), cubeMap->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	UINT passCBByteSize = Utils::CalcBufferByteSize(sizeof(SPassConstants));
	auto resource = OEngine::Get()->CurrentFrameResources->PassCB->GetResource()->GetGPUVirtualAddress();
	for (size_t i = 0; i < cubeMap->GetNumRTVRequired(); i++)
	{
		auto& rtv = cubeMap->GetRTVHandle()[i];
		auto& dsv = cubeMap->GetDSVHandle();
		CHECK(rtv.CPUHandle.ptr != 0);
		CHECK(dsv.CPUHandle.ptr != 0);
		cmdList->ClearRenderTargetView(rtv.CPUHandle, DirectX::Colors::LightSteelBlue, 0, nullptr);
		cmdList->ClearDepthStencilView(dsv.CPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv.CPUHandle, true, &dsv.CPUHandle);
		cmdList->SetGraphicsRootConstantBufferView(1, resource + (i + 1) * passCBByteSize);
		OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::Opaque);
		OEngine::Get()->DrawRenderItems(SPSOType::Sky, SRenderLayer::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cubeMap->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}