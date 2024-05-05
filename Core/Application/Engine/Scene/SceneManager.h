#pragma once
#include "Events.h"
#include "Scene.h"
#include "SceneReader/SceneReader.h"
#include "Types.h"

class OSceneManager
{
public:
	void LoadScenes();
	void Update(UpdateEventArgs& Args);

private:
	OScene* CurrentScene = nullptr;
	unordered_map<string, unique_ptr<OScene>> Scenes;
	unique_ptr<OSceneReader> Reader = nullptr;
};
