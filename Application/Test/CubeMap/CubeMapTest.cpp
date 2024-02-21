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
}

void OCubeMapTest::BuildRenderItems()
{
	auto sky = CreateSphereRenderItem(SRenderLayer::Sky,
	                                  "Sky",
	                                  0.5f,
	                                  20,
	                                  20,
	                                  FindMaterial("Sky"));
	XMStoreFloat4x4(&sky[0].World, DirectX::XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));

	CreateRenderItem(SRenderLayer::Opaque,
	                 "Skull",
	                 "Models/skull.obj",
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

	auto globe = CreateSphereRenderItem(SRenderLayer::Opaque,
	                                    "Globe",
	                                    0.5f,
	                                    20,
	                                    20,
	                                    FindMaterial("Globe"));
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
	                                         { FindMaterial("Cylinder"), 10 });

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