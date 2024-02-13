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
	MaterialsReader = make_unique<OMaterialsReader>(OApplication::Get()->GetConfigPath("MaterialsConfigPath"));
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

void OMaterialManager::LoadMaterials()
{
	Materials = std::move(MaterialsReader->LoadMaterials());
	for (auto& material : Materials)
	{
		FindTexture()
	}
}
