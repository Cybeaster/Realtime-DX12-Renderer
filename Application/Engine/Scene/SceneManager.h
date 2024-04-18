#pragma once
#include "Scene.h"
#include "SceneReader/SceneReader.h"
#include "Types.h"

class OSceneManager
{
public:
	void LoadScenes();

private:
	OScene* CurrentScene = nullptr;
	unordered_map<string, unique_ptr<OScene>> Scenes;
	unique_ptr<OSceneReader> Reader = nullptr;
};
