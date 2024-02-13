#include "MaterialManager.h"

#include "../../Config/ConfigReader.h"
#include "../../Textures/Texture.h"
#include "../../Utils/EngineHelper.h"
#include "Application.h"
#include "Logger.h"
#include "Settings.h"
#include "TextureConstants.h"

void OMaterialManager::CreateMaterial(const string& Name, const int32_t CBIndex, const int32_t HeapIdx, const SMaterialSurface& Surface)
{
	auto mat = make_unique<SMaterial>();
	mat->Name = Name;
	mat->MaterialCBIndex = CBIndex;
	mat->DiffuseSRVHeapIndex = HeapIdx;
	mat->MaterialSurface = Surface;
	AddMaterial(Name, mat);
}

void OMaterialManager::CreateMaterial(const string& Name, STexture* Texture, const SMaterialSurface& Surface)
{
	auto mat = make_unique<SMaterial>();
	mat->Name = Name;
	mat->MaterialCBIndex = Materials.size();
	mat->DiffuseSRVHeapIndex = Texture ? Texture->HeapIdx : FindTextureByName(STextureNames::Debug)->HeapIdx;
	mat->MaterialSurface = Surface;
	AddMaterial(Name, mat);
}

OMaterialManager::OMaterialManager()
{
	MaterialsConfigParser = make_unique<OMaterialsConfigParser>(OApplication::Get()->GetConfigPath("MaterialsConfigPath"));
}

void OMaterialManager::AddMaterial(string Name, unique_ptr<SMaterial>& Material)
{
	if (Materials.contains(Name))
	{
		LOG(Engine, Warning, "Material with this name already exists!");
		return;
	}
	Materials[Name] = move(Material);
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

uint32_t OMaterialManager::GetMaterialCBIndex(const string& Name) const
{
	const auto material = FindMaterial(Name);
	if (!material)
	{
		LOG(Engine, Error, "Material not found!");
		return Name != STextureNames::Debug ? GetMaterialCBIndex(STextureNames::Debug) : -1;
	}
	return material->MaterialCBIndex;
}

uint32_t OMaterialManager::GetNumMaterials() const
{
	return Materials.size();
}

void OMaterialManager::LoadMaterials()
{
	Materials = std::move(MaterialsConfigParser->LoadMaterials());
	for (auto& material : Materials)
	{
		auto& mat = material.second;
		if (mat->DiffuseSRVHeapIndex == -1)
		{
			mat->DiffuseSRVHeapIndex = FindOrCreateTexture(material.second->TexturePath)->HeapIdx;
		}
	}
}

void OMaterialManager::SaveMaterials() const
{
	MaterialsConfigParser->AddMaterials(Materials);
}

void OMaterialManager::BuildMaterialsFromTextures(const std::unordered_map<string, unique_ptr<STexture>>& Textures)
{
	for (auto& texture : Textures)
	{
		CreateMaterial(texture.first, texture.second.get(), SMaterialSurface());
	}
}
