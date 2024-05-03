
#include "CubeMapTest.h"

#include "Application.h"
#include "EngineHelper.h"
#include "Geometry/GPUWave/GpuWave.h"
#include "Window/Window.h"

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

void OCubeMapTest::AnimateSkull(const UpdateEventArgs& Event)
{
}

void OCubeMapTest::BuildRenderItems()
{
	SRenderItemParams skyParams;
	skyParams.MaterialParams = FindMaterial("GrassSky");
	skyParams.NumberOfInstances = 1;
	skyParams.bFrustrumCoolingEnabled = false;
	skyParams.OverrideLayer = SRenderLayers::Sky;
	auto sky = CreateSphereRenderItem("Sky",
	                                  0.5f,
	                                  20,
	                                  20,
	                                  skyParams);
	Scale(sky->Instances[0].World, { 5000.0f, 5000.0f, 5000.0f });

	//auto dragon = CreateRenderItem(SRenderLayer::Opaque, "Dragon", GetResourcePath(L"Resources/Models/dragon/dragon.obj"), EParserType::TinyObjLoader, ETextureMapType::None, SRenderItemParams{ FindMaterial("Bricks") });
	//Scale(dragon->GetDefaultInstance()->World, { 5.f, 5.f, 5.f });
	SRenderItemParams params;
	params.MaterialParams = FindMaterial("Bricks");
	params.NumberOfInstances = 1;
	params.bFrustrumCoolingEnabled = false;
	params.OverrideLayer = SRenderLayers::ShadowDebug;
	auto newQuad = CreateQuadRenderItem("Quad",
	                                    -1.0f,
	                                    1.0f,
	                                    2.0f,
	                                    2.0f,
	                                    0.0f,
	                                    params);

	SRenderItemParams params2;
	params2.MaterialParams = FindMaterial("White");
	params2.NumberOfInstances = 1;
	params2.bFrustrumCoolingEnabled = true;
	params2.Pickable = false;
	params2.Displayable = false;

	//CreateRenderItem("Sponza", GetResourcePath(L"Resources/Models/sponza/sponza.obj"), EParserType::TinyObjLoader, ETextureMapType::None, params2);
	//CreateRenderItem("bistro_int", GetResourcePath(L"Resources/Models/bistro/interior.obj"), EParserType::TinyObjLoader, ETextureMapType::None, params2);
	CreateRenderItem("bistro_ext", GetResourcePath(L"Resources/Models/bistro/exterior.obj"), EParserType::TinyObjLoader, ETextureMapType::None, params2);

	SSpotLightPayload light;
	light.Intensity = { 0.5f, 0.5f, 0.5f };
	light.Direction = { 0.55, -1, 0.055 };
	auto spot = CreateLightSource(light);
	Translate(spot->Instances[0].World, { -3, 3, 0 });

	SDirectionalLightPayload dir;
	dir.Intensity = { 0.5f, 0.5f, 0.5f };
	dir.Direction = { 0.01, -1, 0.01 };

	auto dirlight = CreateLightSource(dir);
	DirLight = Cast<OLightComponent>(dirlight->GetComponents()[0].get());
	/*PointLight point;
	point.Strength = { 0.5f, 0.5f, 0.5f };
	auto pointLight = CreateLightSource(SRenderLayer::LightObjects, point);
	Translate(pointLight->Instances[0].World, { 3, 3, 0 });
	*/

	Water = OEngine::Get()->BuildRenderObject<OWaterRenderObject>(ERenderGroup::RenderTargets);
}