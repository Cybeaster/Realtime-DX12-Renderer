
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
	skyParams.MaterialParams = FindMaterial("GrassSky");
	skyParams.NumberOfInstances = 1;
	skyParams.bFrustrumCoolingEnabled = false;
	skyParams.OverrideLayer = SRenderLayers::Sky;
	skyParams.Scale = { 5000.0f, 5000.0f, 5000.0f };

	CreateSphereRenderItem("Sky",
	                       0.5f,
	                       20,
	                       20,
	                       skyParams);

	SRenderItemParams objectsParams;
	objectsParams.MaterialParams = FindMaterial("White");
	objectsParams.NumberOfInstances = 1;
	objectsParams.bFrustrumCoolingEnabled = true;
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

	SRenderItemParams params;
	params.MaterialParams = FindMaterial("Bricks");
	params.NumberOfInstances = 1;
	params.bFrustrumCoolingEnabled = false;
	params.OverrideLayer = SRenderLayers::ShadowDebug;
	CreateQuadRenderItem("Quad",
	                     -1.0f,
	                     1.0f,
	                     2.0f,
	                     2.0f,
	                     0.0f,
	                     params);

	OEngine::Get()->BuildRenderObject<OWaterRenderObject>(ERenderGroup::RenderTargets);
}