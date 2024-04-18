#include "SceneReader.h"

#include "DirectX/Light/Light.h"

SSceneSettings OSceneReader::LoadScene()
{
	SSceneSettings res;

	res.CurrentScene = PTree.get_child("Scene").get<string>("CurrentScene");

	for (const auto& scene : PTree.get_child("Scenes") | std::views::values)
	{
		SScenePayload payload;
		payload.Name = scene.get<string>("Name");
		payload.Skybox = scene.get<string>("Sky");

		for (auto obj : scene.get_child("Objects"))
		{
			payload.Objects.push_back(obj.second.get_value<string>());
		}

		for (auto light : scene.get_child("Lights"))
		{
			string type = light.second.get<string>("Type");
			if (type == "Directional")
			{
				SDirectionalLightPayload dirLight;
				dirLight.Direction = {
					light.second.get_child("Direction").get<float>("X"),
					light.second.get_child("Direction").get<float>("Y"),
					light.second.get_child("Direction").get<float>("Z")
				};
				dirLight.Strength = {
					light.second.get_child("Color").get<float>("R"),
					light.second.get_child("Color").get<float>("G"),
					light.second.get_child("Color").get<float>("B")
				};
				payload.DirLights.push_back(dirLight);
			}
		}
		res.Scenes.push_back(payload);
	}
	return res;
}