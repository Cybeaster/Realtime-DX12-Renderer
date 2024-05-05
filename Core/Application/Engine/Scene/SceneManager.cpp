#include "SceneManager.h"

#include "Application.h"

void OSceneManager::LoadScenes()
{
	if (!Reader)
	{
		Reader = make_unique<OSceneReader>(OApplication::Get()->GetConfigPath("SceneConfigPath"));
	}
	const auto scene = Reader->LoadScene();
	for (const auto& scenePayload : scene.Scenes)
	{
		unique_ptr<OScene> newScene = make_unique<OScene>();
		SSceneInfo info;
		info.Name = scenePayload.Name;
		info.Skybox = scenePayload.Skybox;
		info.Objects = scenePayload.Objects;
		info.DirLights = scenePayload.DirLights;
		info.SpotLights = scenePayload.SpotLights;
		if (info.Name == scene.CurrentScene)
		{
			CurrentScene = newScene.get();
		}
		newScene->InitScene(info);
		Scenes[scenePayload.Name] = move(newScene);
	}
	if (CurrentScene)
	{
		CurrentScene->Load();
	}
	else
	{
		LOG(Engine, Error, "Current scene not found")
	}
}

void OSceneManager::Update(UpdateEventArgs& Args)
{
}