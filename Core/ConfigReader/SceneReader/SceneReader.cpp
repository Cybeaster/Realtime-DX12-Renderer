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

		for (auto obj : scene.get_child("Objects") | std::views::values)
		{
			payload.Objects.push_back(obj.get<string>("ObjectName"));
		}

		for (auto light : scene.get_child("Lights"))
		{
			string type = light.second.get<string>("Type");
			if (type == "Directional")
			{
				SDirectionalLightPayload dirLight;
				GetFloat3(light.second, "Direction", dirLight.Direction); //todo optimize boilerplate
				dirLight.Intensity = {
					light.second.get_child("Intensity").get<float>("R"),
					light.second.get_child("Intensity").get<float>("G"),
					light.second.get_child("Intensity").get<float>("B")
				};
				payload.DirLights.push_back(dirLight);
			}
			else if (type == "Spot")
			{
				SSpotLightPayload spotLight;
				GetFloat3(light.second, "Direction", spotLight.Direction);
				spotLight.Intensity = {
					light.second.get_child("Intensity").get<float>("R"),
					light.second.get_child("Intensity").get<float>("G"),
					light.second.get_child("Intensity").get<float>("B")
				};
				//Read position
				payload.SpotLights.push_back(spotLight);
			}
		}
		res.Scenes.push_back(payload);
	}
	return res;
}