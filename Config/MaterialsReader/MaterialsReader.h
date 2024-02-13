#pragma once
#include "../../Materials/Material.h"
#include "../ConfigReader.h"

#include <ranges>
class OMaterialsReader : public OConfigReader
{
public:
	OMaterialsReader(const string& ConfigPath)
	    : OConfigReader(ConfigPath) {}

	std::unordered_map<string, unique_ptr<SMaterial>> LoadMaterials()
	{
		std::unordered_map<string, unique_ptr<SMaterial>> Materials;
		LoadConfig(FileName);
		for (const auto& val : PTree.get_child("Materials") | std::views::values)
		{
			auto material = make_unique<SMaterial>();
			material->Name = val.get<string>("Name");
			auto& data = val.get_child("Data");

			material->TexturePath = data.get<string>("TexturePath");

			//Load diffuse
			float diffuse[4];
			int index = 0;
			for (const auto& val : data.get_child("Diffuse") | std::views::values)
			{
				diffuse[index] = val.get_value<float>();
				index++;
			}
			material->MaterialSurface.DiffuseAlbedo = { diffuse[0], diffuse[1], diffuse[2], diffuse[3] };

			// Load Fresnel
			index = 0;
			float fresnel[3];
			for (const auto& val : data.get_child("Fresnel") | std::views::values)
			{
				fresnel[index] = val.get_value<float>();
			}
			material->MaterialSurface.FresnelR0 = { fresnel[0], fresnel[1], fresnel[2] };
			material->MaterialSurface.Roughness = data.get<float>("Roughness");
			Materials[material->Name] = std::move(material);
		}
		return std::move(Materials);
	}

	/*Adds or modifies the material in the tree*/
	void AddMaterial(const unique_ptr<SMaterial>& Material)
	{
		LoadConfig(FileName);
		auto name = Material->Name;

		for (auto& item : PTree.get_child("Materials"))
		{
			if (item.second.get<string>("Name") == name)
			{
				auto data = item.second.get_child("Data");

				float diffuse[4] = { Material->MaterialSurface.DiffuseAlbedo.x,
					                 Material->MaterialSurface.DiffuseAlbedo.y,
					                 Material->MaterialSurface.DiffuseAlbedo.z,
					                 Material->MaterialSurface.DiffuseAlbedo.w };

				float fresnel[3] = { Material->MaterialSurface.FresnelR0.x,
					                 Material->MaterialSurface.FresnelR0.y,
					                 Material->MaterialSurface.FresnelR0.z };

				data.put("Name", Material->Name);
				data.put("TexturePath", Material->TexturePath);
				data.put("Diffuse", diffuse);
				data.put("Fresnel", fresnel);
				data.put("Roughness", Material->MaterialSurface.Roughness);

				return;
			}
		}
	}

	void AddMaterials(const map<string, vector<unique_ptr<SMaterial>>>& Materials)
	{
		for (const auto& value : Materials | std::views::values)
		{
			for (const auto& material : value)
			{
				AddMaterial(material);
			}
		}
	}
};