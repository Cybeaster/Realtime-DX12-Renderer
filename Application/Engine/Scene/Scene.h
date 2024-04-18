#pragma once
#include "DirectX/Light/Light.h"
#include "Types.h"

struct SSceneInfo
{
	string Name;
	string Skybox;
	vector<string> Objects;
	vector<SDirectionalLightPayload> DirLights;
	vector<SSpotLightPayload> SpotLights;
};

class OScene
{
public:
	void InitScene(const SSceneInfo& SceneInfo);
	void Load() const;

private:
	SSceneInfo Info;
};
