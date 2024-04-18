
#include "Scene.h"

#include "EngineHelper.h"
void OScene::InitScene(const SSceneInfo& SceneInfo)
{
	Info = SceneInfo;
}

void OScene::Load() const
{
	//create objects
	for (auto& obj : Info.Objects)
	{
		auto path = std::filesystem::path(GetResourcePath(UTF8ToWString(obj)));
		if (exists(path))
		{
			CreateRenderItem(path.filename().string(), path.wstring(), EParserType::TinyObjLoader, ETextureMapType::None, SRenderItemParams{});
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
}