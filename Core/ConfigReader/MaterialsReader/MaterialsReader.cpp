#include "MaterialsReader.h"

vector<STexturePath> OMaterialsConfigParser::GetTexturePaths(const boost::property_tree::ptree& Node, const string& Key)
{
	vector<STexturePath> paths;
	for (const auto& val : Node.get_child(Key) | std::views::values)
	{
		STexturePath path;
		path.Path = UTF8ToWString(val.data());
		paths.push_back(path);
	}
	return paths;
}

std::unordered_map<string, shared_ptr<SMaterial>> OMaterialsConfigParser::LoadMaterials()
{
	std::unordered_map<string, shared_ptr<SMaterial>> Materials;
	LoadConfig(FileName);
	for (const auto& val : PTree.get_child("Materials") | std::views::values)
	{
		auto material = make_shared<SMaterial>();
		material->Name = val.get<string>("Name");
		auto& data = val.get_child("Data");

		auto diffuse = GetTexturePaths(data, "DiffuseMapPaths");
		auto normal = GetTexturePaths(data, "NormalMapPaths");
		auto height = GetTexturePaths(data, "HeightMapPaths");

		material->DiffuseMap = !diffuse.empty() ? diffuse.at(0) : STexturePath();
		material->NormalMap = !normal.empty() ? normal.at(0) : STexturePath();
		material->HeightMap = !height.empty() ? height.at(0) : STexturePath();

		auto& diffChild = data.get_child("Diffuse");
		material->MaterialData.DiffuseAlbedo.x = diffChild.get<float>("x");
		material->MaterialData.DiffuseAlbedo.y = diffChild.get<float>("y");
		material->MaterialData.DiffuseAlbedo.z = diffChild.get<float>("z");

		material->MaterialData.Roughness = data.get<float>("Roughness");
		material->MaterialData.Reflection = data.get_optional<float>("Reflection").value_or(0.0f);
		Materials[material->Name] = std::move(material);
	}
	return std::move(Materials);
}

void OMaterialsConfigParser::AddDataToNode(const SMaterial* Mat, boost::property_tree::ptree& OutNode)
{
	OutNode.put("Diffuse.x", Mat->MaterialData.DiffuseAlbedo.x);
	OutNode.put("Diffuse.y", Mat->MaterialData.DiffuseAlbedo.y);
	OutNode.put("Diffuse.z", Mat->MaterialData.DiffuseAlbedo.z);

	OutNode.put("Roughness", Mat->MaterialData.Roughness);
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
