#pragma once
#include "ConfigReader.h"
#include "DirectX/Light/Light.h"

struct SScenePayload
{
	string Name;
	vector<string> Objects;
	vector<SDirectionalLightPayload> DirLights;
	vector<SSpotLightPayload> SpotLights;
	string Skybox;
};

struct SSceneSettings
{
	string CurrentScene;
	vector<SScenePayload> Scenes;
};

class OSceneReader : public OConfigReader
{
public:
	OSceneReader(const string& ConfigPath)
	    : OConfigReader(ConfigPath)
	{
	}

	SSceneSettings LoadScene();
};
