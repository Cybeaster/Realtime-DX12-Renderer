
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
	if (DirLight)
	{
		SDirectionalLight light = DirLight->GetDirectionalLight();
		light.Direction.x = sin(Event.Timer.GetTime() * 0.2f);
		light.Direction.z = cos(Event.Timer.GetTime() * 0.2f);
		SDirectionalLightPayload payload;
		payload.Direction = light.Direction;
		payload.Strength = light.Strength;
		DirLight->SetDirectionalLight(payload);
	}
}

void OCubeMapTest::AnimateSkull(const UpdateEventArgs& Event)
{
}

void OCubeMapTest::BuildRenderItems()
{
	auto sky = CreateSphereRenderItem(SRenderLayer::Sky,
	                                  "Sky",
	                                  0.5f,
	                                  20,
	                                  20,
	                                  FindMaterial("GrassSky"));
	Scale(sky->Instances[0].World, { 5000.0f, 5000.0f, 5000.0f });

	//auto dragon = CreateRenderItem(SRenderLayer::Opaque, "Dragon", GetResourcePath(L"Resources/Models/dragon/dragon.obj"), EParserType::TinyObjLoader, ETextureMapType::None, SRenderItemParams{ FindMaterial("Bricks") });
	//Scale(dragon->GetDefaultInstance()->World, { 5.f, 5.f, 5.f });
	SRenderItemParams params;
	params.MaterialParams = FindMaterial("Bricks");
	params.NumberOfInstances = 1;
	params.bFrustrumCoolingEnabled = false;
	auto newQuad = CreateQuadRenderItem(SRenderLayer::ShadowDebug,
	                                    "Quad",
	                                    -1.0f,
	                                    1.0f,
	                                    2.0f,
	                                    2.0f,
	                                    0.0f,
	                                    params);

	SRenderItemParams params2;
	params2.MaterialParams = FindMaterial("White");
	params2.NumberOfInstances = 1;
	params2.bFrustrumCoolingEnabled = false;
	params2.Pickable = false;
	auto room = CreateRenderItem(SRenderLayer::Opaque, "Sponza", GetResourcePath(L"Resources/Models/sponza/sponza.obj"), EParserType::TinyObjLoader, ETextureMapType::None, params2);

	SSpotLightPayload light;
	light.Strength = { 0.5f, 0.5f, 0.5f };
	light.Direction = { 0.55, -1, 0.055 };
	auto spot = CreateLightSource(SRenderLayer::LightObjects,
	                              light);
	Translate(spot->Instances[0].World, { -3, 3, 0 });

	SDirectionalLightPayload dir;
	dir.Strength = { 0.5f, 0.5f, 0.5f };
	dir.Direction = { 0.01, -1, 0.01 };

	auto dirlight = CreateLightSource(SRenderLayer::LightObjects, dir);
	DirLight = Cast<OLightComponent>(dirlight->GetComponents()[0].get());
	/*PointLight point;
point.Strength = { 0.5f, 0.5f, 0.5f };
auto pointLight = CreateLightSource(SRenderLayer::LightObjects, point);
Translate(pointLight->Instances[0].World, { 3, 3, 0 });
*/

	Water = OEngine::Get()->BuildRenderObject<OWaterRenderObject>(ERenderGroup::RenderTargets);
}