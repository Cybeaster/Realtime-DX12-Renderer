#include "MaterialManager.h"

#include "Application.h"
#include "EngineHelper.h"
#include "Logger.h"
#include "PathUtils.h"
#include "TextureConstants.h"

#include <future>

void OMaterialManager::CreateMaterial(const string& Name, STexture* Texture, const SMaterialSurface& Surface, bool Notify /*= false*/)
{
	auto mat = make_unique<SMaterial>();
	mat->Name = Name;
	mat->MaterialCBIndex = Materials.size();
	mat->MaterialSurface = Surface;
	AddMaterial(Name, std::move(mat), Notify);
}

SMaterial* OMaterialManager::CreateMaterial(const SMaterialPayloadData& Data)
{
	auto name = Data.Name;
	if (name.empty() || (Data.DiffuseMaps.size() == 0 && Data.NormalMaps.size() == 0 && Data.HeightMaps.size() == 0))
	{
		LOG(Engine, Warning, "Material is empty!");
		return nullptr;
	}

	if (Materials.contains(name))
	{
		LOG(Engine, Warning, "Material with name {} already exists!", TEXT(name));
		//name = name + "_" + std::to_string(Materials.size());
		return Materials.at(name).get();
	}

	auto mat = make_unique<SMaterial>();
	auto res = mat.get();
	mat->Name = name;
	mat->MaterialCBIndex = Materials.size();
	CWIN_LOG(mat->MaterialCBIndex > SRenderConstants::Max2DTextures, Material, Error, "Material index is out of range!");
	mat->MaterialSurface = Data.MaterialSurface;
	mat->DiffuseMaps = LoadTexturesFromPaths(Data.DiffuseMaps);
	mat->NormalMaps = LoadTexturesFromPaths(Data.NormalMaps);
	mat->HeightMaps = LoadTexturesFromPaths(Data.HeightMaps);
	AddMaterial(name, std::move(mat));
	return res;
}

OMaterialManager::OMaterialManager()
{
	MaterialsConfigParser = make_unique<OMaterialsConfigParser>(OApplication::Get()->GetConfigPath("MaterialsConfigPath"));
}

void OMaterialManager::AddMaterial(string Name, unique_ptr<SMaterial> Material, bool Notify /*= false*/)
{
	if (Materials.contains(Name))
	{
		LOG(Engine, Error, "Material with name {} already exists!", TEXT(Name));
		return;
	}

	MaterialsIndicesMap[Material->MaterialCBIndex] = Material.get();
	Materials[Name] = move(Material);
	if (Notify)
	{
		MaterialsRebuld.Broadcast();
	}
}

const OMaterialManager::TMaterialsMap& OMaterialManager::GetMaterials() const
{
	return Materials;
}

SMaterial* OMaterialManager::FindMaterial(const string& Name) const
{
	if (!Materials.contains(Name))
	{
		LOG(Engine, Error, "Material not found!");
		return Name != STextureNames::Debug ? FindMaterial(STextureNames::Debug) : nullptr;
		;
	}
	return Materials.at(Name).get();
}

SMaterial* OMaterialManager::FindMaterial(const uint32_t Index) const
{
	if (!MaterialsIndicesMap.contains(Index))
	{
		LOG(Engine, Error, "Material not found!");
		return FindMaterial(STextureNames::Debug);
	}
	return MaterialsIndicesMap.at(Index);
}

uint32_t OMaterialManager::GetMaterialCBIndex(const string& Name)
{
	const auto material = FindMaterial(Name);
	if (!material)
	{
		LOG(Engine, Error, "Material not found!");
		return Name != STextureNames::Debug ? GetMaterialCBIndex(STextureNames::Debug) : -1;
	}
	return material->MaterialCBIndex;
}

uint32_t OMaterialManager::GetNumMaterials()
{
	return Materials.size();
}

vector<STexturePath> OMaterialManager::LoadTexturesFromPaths(const vector<wstring>& Paths)
{
	vector<STexturePath> result;
	for (const auto& str : Paths)
	{
		if (str.empty())
		{
			continue;
		}
		std::filesystem::path path(str);
		if (!std::filesystem::exists(path))
		{
			LOG(Engine, Warning, "Texture file not found!");
			continue;
		}

		if (path.extension() != ".dds")
		{
			path.replace_extension(".dds");
		}
		auto filename = path.filename().string();
		STexturePath texturePath;
		texturePath.Path = path.c_str();
		texturePath.Texture = FindOrCreateTexture(filename, path);
		if (!texturePath.Texture)
		{
			LOG(Engine, Warning, "Texture with path {} not found!", TEXT(path));
		}
		result.push_back(texturePath);
	}
	return result;
}

void OMaterialManager::LoadMaterialsFromCache()
{
	auto materials = std::move(MaterialsConfigParser->LoadMaterials());
	uint32_t it = 0;
	for (auto& val : materials | std::views::values)
	{
		auto& mat = val;
		LoadTexturesFromPaths(mat->DiffuseMaps);
		LoadTexturesFromPaths(mat->NormalMaps);
		LoadTexturesFromPaths(mat->HeightMaps);
		mat->MaterialCBIndex = it;
		MaterialsIndicesMap[it] = val.get();
		++it;
		auto name = mat->Name;
		AddMaterial(name, std::move(val), false);
	}
	MaterialsRebuld.Broadcast();
}

void OMaterialManager::LoadTexturesFromPaths(vector<STexturePath>& OutTextures)
{
	for (auto& [Texture, Path] : OutTextures)
	{
		Texture = FindOrCreateTexture(Path);
		if (!Texture)
		{
			LOG(Engine, Warning, "Texture with path {} not found!", Path);
		}
	}
}

void OMaterialManager::SaveMaterials() const
{
	//TODO possible data run
	std::unordered_map<string, SMaterial*> materials;
	for (const auto& [fst, snd] : this->Materials)
	{
		materials[fst] = snd.get();
	}

	std::thread([&, localMat = std::move(materials)]() {
		MaterialsConfigParser->AddMaterials(localMat);
	}).detach();
}

void OMaterialManager::BuildMaterialsFromTextures(const std::unordered_map<string, unique_ptr<STexture>>& Textures)
{
	for (auto& texture : Textures)
	{
		CreateMaterial(texture.first, texture.second.get(), SMaterialSurface());
	}
	MaterialsRebuld.Broadcast();
}

void OMaterialManager::OnMaterialChanged(const string& Name)
{
	Materials[Name]->NumFramesDirty = SRenderConstants::NumFrameResources;
}
