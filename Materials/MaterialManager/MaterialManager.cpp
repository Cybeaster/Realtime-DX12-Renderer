#include "MaterialManager.h"

#include "Application.h"
#include "EngineHelper.h"
#include "Logger.h"
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
	auto mat = make_unique<SMaterial>();
	mat->Name = Data.Name;
	mat->MaterialCBIndex = Materials.size();
	mat->MaterialSurface = Data.MaterialSurface;
	mat->DiffuseMaps = LoadTexturesFromPaths(Data.DiffuseMaps);
	mat->NormalMaps = LoadTexturesFromPaths(Data.NormalMaps);
	mat->HeightMaps = LoadTexturesFromPaths(Data.HeightMaps);
	AddMaterial(Data.Name, std::move(mat));
	return mat.get();
}

OMaterialManager::OMaterialManager()
{
	MaterialsConfigParser = make_unique<OMaterialsConfigParser>(OApplication::Get()->GetConfigPath("MaterialsConfigPath"));
}

void OMaterialManager::AddMaterial(string Name, unique_ptr<SMaterial> Material, bool Notify /*= false*/)
{
	if (Materials.contains(Name))
	{
		LOG(Engine, Warning, "Material with this name already exists!");
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
	for (const auto& path : Paths)
	{
		STexturePath texturePath;
		texturePath.Path = path;
		texturePath.Texture = FindOrCreateTexture(path);
		if (!texturePath.Texture)
		{
			LOG(Engine, Warning, "Texture with path {} not found!", path);
		}
		result.push_back(texturePath);
	}
	return result;
}

void OMaterialManager::LoadMaterialsFromCache()
{
	Materials = std::move(MaterialsConfigParser->LoadMaterials());
	uint32_t it = 0;
	for (auto& val : Materials | std::views::values)
	{
		auto& mat = val;
		LoadTexturesFromPaths(mat->DiffuseMaps);
		LoadTexturesFromPaths(mat->NormalMaps);
		LoadTexturesFromPaths(mat->HeightMaps);
		mat->MaterialCBIndex = it;
		MaterialsIndicesMap[it] = val.get();
		++it;
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
