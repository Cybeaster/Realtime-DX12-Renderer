//
// Created by Cybea on 21/02/2024.
//

#include "CubeMapTest.h"

#include "../../../Objects/Geometry/GPUWave/GpuWave.h"
#include "../../../Utils/EngineHelper.h"
bool OCubeMapTest::Initialize()
{
	OEngine::Get()->BuildCubeRenderTarget({ 0, 2, 0 });
	auto queue = OEngine::Get()->GetCommandQueue();
	Waves = OEngine::Get()->BuildRenderObject<OGPUWave>(OEngine::Get()->GetDevice().Get(),
	                                                    queue->GetCommandList().Get(),
	                                                    256,
	                                                    256,
	                                                    0.25f,
	                                                    0.03f,
	                                                    2.0f,
	                                                    0.2f);
	BuildRenderItems();
	return true;
}

void OCubeMapTest::OnUpdate(const UpdateEventArgs& Event)
{
	OTest::OnUpdate(Event);
	AnimateSkull(Event);
}

void OCubeMapTest::DrawSceneToCubeMap()
{
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList();

	auto cubeMap = OEngine::Get()->GetCubeRenderTarget();

	cmdList->SetGraphicsRootDescriptorTable(5, OEngine::Get()->GetSRVDescHandleForTexture(FindTextureByName("grasscube1024")));

	cubeMap->SetViewport(OEngine::Get()->GetCommandQueue());

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

		cmdList->SetGraphicsRootConstantBufferView(2, resource + (i + 1) * passCBByteSize);

		OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::Opaque);
		OEngine::Get()->DrawRenderItems(SPSOType::WavesRender, SRenderLayer::Waves);
		OEngine::Get()->DrawRenderItems(SPSOType::Sky, SRenderLayer::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cubeMap->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OCubeMapTest::OnRender(const UpdateEventArgs& Event)
{
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList();
	cmdList->SetGraphicsRootDescriptorTable(4, Waves->GetDisplacementMapHandle());

	DrawSceneToCubeMap();
	OEngine::Get()->GetWindow()->SetViewport(OEngine::Get()->GetCommandQueue());
	GetEngine()->PrepareRenderTarget();

	auto passCB = OEngine::Get()->CurrentFrameResources->PassCB->GetResource();
	cmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	auto cubeMap = OEngine::Get()->GetCubeRenderTarget();
	cmdList->SetGraphicsRootDescriptorTable(5, cubeMap->GetSRVHandle().GPUHandle);

	OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::OpaqueDynamicReflections);

	cmdList->SetGraphicsRootDescriptorTable(5, OEngine::Get()->GetSRVDescHandleForTexture(FindTextureByName("grasscube1024")));

	OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::Opaque);

	OEngine::Get()->DrawRenderItems(SPSOType::WavesRender, SRenderLayer::Waves);

	OEngine::Get()->DrawRenderItems(SPSOType::Sky, SRenderLayer::Sky);
}

void OCubeMapTest::AnimateSkull(const UpdateEventArgs& Event)
{
	using namespace DirectX;
	XMMATRIX skullScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	XMMATRIX skullOffset = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	XMMATRIX skullLocalRotate = XMMatrixRotationY(2.0f * Event.Timer.GetTime());
	XMMATRIX skullGlobalRotate = XMMatrixRotationY(0.5f * Event.Timer.GetTime());
	XMStoreFloat4x4(&SkullRitem->World, skullScale * skullLocalRotate * skullOffset * skullGlobalRotate);
}

void OCubeMapTest::BuildRenderItems()
{
	auto& sky = CreateSphereRenderItem(SRenderLayer::Sky,
	                                   "Sky",
	                                   0.5f,
	                                   20,
	                                   20,
	                                   FindMaterial("GrassSky"));
	Scale(sky[0].World, { 5000.0f, 5000.0f, 5000.0f });

	auto& skull = CreateRenderItem(SRenderLayer::Opaque,
	                               "Skull",
	                               "Resources/Models/skull.txt",
	                               EParserType::Custom,
	                               ETextureMapType::Spherical,
	                               SRenderItemParams{ FindMaterial("White") });
	Scale(skull[0].World, { 0.5f, 0.5f, 0.5f });
	SkullRitem = &skull[0];

	auto& box = CreateBoxRenderItem(SRenderLayer::Opaque,
	                                "Box",
	                                1.0f,
	                                1.0f,
	                                1.0f,
	                                3,
	                                FindMaterial("Tile"));
	box[0].World = Scale(Translate(box[0].World, { 2.0f, 1.0f, 2.0f }), { 1.0f, 2.0f, 1.0f });

	auto& globe = CreateSphereRenderItem(SRenderLayer::OpaqueDynamicReflections,
	                                     "Globe",
	                                     0.5f,
	                                     20,
	                                     20,
	                                     FindMaterial("Mirror"));

	Scale(globe[0].World, { 3, 3, 3 });
	Translate(globe[0].World, { 0, 3, 0 });

	auto& grid
	    = CreateGridRenderItem(SRenderLayer::Opaque,
	                           "Grid",
	                           20.0f,
	                           30.0f,
	                           60,
	                           40,
	                           FindMaterial("Tile"));
	Scale(grid[0].TexTransform, { 8, 8, 1.0f });

	auto& cylinder = CreateCylinderRenderItem(SRenderLayer::Opaque,
	                                          "Cylinder",
	                                          0.5f,
	                                          0.5f,
	                                          3.0f,
	                                          20,
	                                          20,
	                                          { FindMaterial("Bricks"), 10 });

	auto& spheres = CreateSphereRenderItem(SRenderLayer::Opaque,
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

		auto& leftSphere = spheres[it];
		auto& rightSphere = spheres[it + 1];

		auto& leftCylinder = cylinder[it];
		auto& rightCylinder = cylinder[it + 1];

		Put(leftSphere.World, leftSphereWorld);
		Put(rightSphere.World, rightSphereWorld);
		Put(rightCylinder.TexTransform, brickTexTransform);
		Put(leftCylinder.TexTransform, brickTexTransform);
		Put(leftCylinder.World, leftCylWorld);
		Put(rightCylinder.World, rightCylWorld);
	}

	CreateGridRenderItem(SRenderLayer::Waves,
	                     "Water",
	                     160,
	                     160,
	                     Waves->GetRowCount(),
	                     Waves->GetColumnCount(),
	                     Waves->GetRIParams());
}