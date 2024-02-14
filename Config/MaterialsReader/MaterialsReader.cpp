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

		auto& diffChild = data.get_child("Diffuse");
		material->MaterialSurface.DiffuseAlbedo.x = diffChild.get<float>("x");
		material->MaterialSurface.DiffuseAlbedo.y = diffChild.get<float>("y");
		material->MaterialSurface.DiffuseAlbedo.z = diffChild.get<float>("z");
		material->MaterialSurface.DiffuseAlbedo.w = diffChild.get<float>("w");

		auto& fresnelChild = data.get_child("Fresnel");
		material->MaterialSurface.FresnelR0.x = fresnelChild.get<float>("x");
		material->MaterialSurface.FresnelR0.y = fresnelChild.get<float>("y");
		material->MaterialSurface.FresnelR0.z = fresnelChild.get<float>("z");

		material->MaterialSurface.Roughness = data.get<float>("Roughness");
		Materials[material->Name] = std::move(material);
	}
	return std::move(Materials);
}

void OMaterialsConfigParser::AddDataToNode(const SMaterial* Mat, boost::property_tree::ptree& OutNode)
{
	ENSURE(Mat->TexturePath.size() > 0);
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

void OMaterialsConfigParser::AddMaterial(const SMaterial* Material)
{
	using namespace boost::property_tree;
	LoadConfig(FileName);
	auto name = Material->Name;

	bool foundMat = false;
	auto& items = PTree.get_child("Materials");
	for (auto& item : items)
	{
		if (item.second.get<string>("Name") == name)
		{
			auto& matNode = item.second;
			auto& data = matNode.get_child("Data");
			matNode.put("Name", Material->Name);
			AddDataToNode(Material, data);
			foundMat = true;
			break;
		}
	}

	if (!foundMat)
	{
		ptree matNode;
		matNode.put("Name", Material->Name);
		ptree data;
		AddDataToNode(Material, data);
		matNode.add_child("Data", data);
		items.push_back(std::make_pair("", matNode));
	}
	write_json(FileName, PTree);
}

void OMaterialsConfigParser::AddMaterials(const std::unordered_map<string, SMaterial*>& Materials)
{
	for (const auto& value : Materials | std::views::values)
	{
		AddMaterial(value);
	}
}