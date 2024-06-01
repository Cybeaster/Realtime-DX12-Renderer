
#include "Scene.h"

#include "EngineHelper.h"
#include "Geometry/Water/Water.h"
void OScene::InitScene(const SSceneInfo& SceneInfo)
{
	Info = SceneInfo;
}

void OScene::Load() const
{
	OEngine::Get()->BuildCubeRenderTarget({ 0, 2, 0 });

	//create objects

	SRenderItemParams skyParams;
	skyParams.Material = FindMaterial("GrassSky");
	skyParams.NumberOfInstances = 1;
	skyParams.bFrustumCoolingEnabled = false;
	skyParams.OverrideLayer = SRenderLayers::Sky;
	skyParams.Scale = { 5000.0f, 5000.0f, 5000.0f };

	CreateSphereRenderItem("Sky",
	                       0.5f,
	                       20,
	                       20,
	                       skyParams);

	SRenderItemParams objectsParams;
	objectsParams.Material = FindMaterial("White");
	objectsParams.NumberOfInstances = 1;
	objectsParams.bFrustumCoolingEnabled = true;
	objectsParams.Pickable = false;
	objectsParams.Displayable = false;

	for (auto& obj : Info.Objects)
	{
		auto path = std::filesystem::path(GetResourcePath(UTF8ToWString(obj)));
		if (exists(path))
		{
			CreateRenderItem(path.filename().string(), path.wstring(), EParserType::TinyObjLoader, ETextureMapType::None, objectsParams);
		}
		else
		{
			WIN_LOG(Engine, Error, "Object not found: {}", TEXT(obj));
		}
	}

	for (const auto& dirLight : Info.DirLights)
	{
		CreateLightSource(dirLight);
	}

	for (const auto& spotLights : Info.SpotLights)
	{
		CreateLightSource(spotLights);
	}

	SRenderItemParams params;
	params.Material = FindMaterial("White");
	params.NumberOfInstances = 1;
	params.bFrustumCoolingEnabled = false;
	params.OverrideLayer = SRenderLayers::ShadowDebug;
	params.Pickable = false;
	params.Displayable = false;

	CreateQuadRenderItem("Quad",
	                     -1.0f,
	                     1.0f,
	                     2.0f,
	                     2.0f,
	                     0.0f,
	                     params);

	OEngine::Get()->BuildRenderObject<OWaterRenderObject>(ERenderGroup::RenderTargets);
}