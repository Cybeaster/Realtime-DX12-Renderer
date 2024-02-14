#include "MaterialsReader.h"

std::unordered_map<string, unique_ptr<SMaterial>> OMaterialsConfigParser::LoadMaterials()
{
	std::unordered_map<string, unique_ptr<SMaterial>> Materials;
	LoadConfig(FileName);
	for (const auto& val : PTree.get_child("Materials") | std::views::values)
	{
		auto material = make_unique<SMaterial>();
		material->Name = val.get<string>("Name");
		auto& data = val.get_child("Data");

		material->TexturePath = UTF8ToWString(data.get<string>("TexturePath"));

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

void OMaterialsConfigParser::AddDataToNode(SMaterial* Mat, boost::property_tree::ptree& OutNode)
{
	OutNode.put("TexturePath", WStringToUTF8(Mat->TexturePath));

	OutNode.put("Diffuse.x", Mat->MaterialSurface.DiffuseAlbedo.x);
	OutNode.put("Diffuse.y", Mat->MaterialSurface.DiffuseAlbedo.y);
	OutNode.put("Diffuse.z", Mat->MaterialSurface.DiffuseAlbedo.z);
	OutNode.put("Diffuse.w", Mat->MaterialSurface.DiffuseAlbedo.w);

	OutNode.put("Fresnel.x", Mat->MaterialSurface.FresnelR0.x);
	OutNode.put("Fresnel.y", Mat->MaterialSurface.FresnelR0.y);
	OutNode.put("Fresnel.z", Mat->MaterialSurface.FresnelR0.z);
	OutNode.put("Roughness", Mat->MaterialSurface.Roughness);
}

void OMaterialsConfigParser::AddMaterial(const unique_ptr<SMaterial>& Material)
{
	using namespace boost::property_tree;
	LoadConfig(FileName);
	auto name = Material->Name;

	ptree matNode;
	bool foundMat = false;
	auto& items = PTree.get_child("Materials");
	for (auto& item : items)
	{
		if (item.second.get<string>("Name") == name)
		{
			matNode = item.second;
			auto data = matNode.get_child("Data");
			matNode.put("Name", Material->Name);
			AddDataToNode(Material.get(), data);
			foundMat = true;
			break;
		}
	}

	if (!foundMat)
	{
		matNode.put("Name", Material->Name);
		ptree data;
		AddDataToNode(Material.get(), data);
		matNode.add_child("Data", data);
		items.push_back(std::make_pair("", matNode));
	}
	write_json(FileName, PTree);
}

void OMaterialsConfigParser::AddMaterials(const std::unordered_map<string, unique_ptr<SMaterial>>& Materials)
{
	for (const auto& value : Materials | std::views::values)
	{
		AddMaterial(value);
	}
}