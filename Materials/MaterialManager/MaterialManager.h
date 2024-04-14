#pragma once
#include "Async.h"
#include "DirectX/MaterialData.h"
#include "Events.h"
#include "Material.h"
#include "MaterialsReader/MaterialsReader.h"
#include "Types.h"

#include <unordered_map>

struct STexture;

class OMaterialManager
{
public:
	DECLARE_DELEGATE(OnMaterialsChanged);

	OMaterialManager();
	using TMaterialsMap = std::unordered_map<string, unique_ptr<SMaterial>>;
	using TMaterialsIndexMap = std::unordered_map<uint32_t, SMaterial*>;
	void AddMaterial(string Name, unique_ptr<SMaterial> Material, bool Notify = false /*= false*/);
	void CreateMaterial(const string& Name, STexture* Texture, const SMaterialSurface& Surface, bool Notify = false /*= false*/);
	SMaterial* CreateMaterial(const SMaterialPayloadData& Data);
	const TMaterialsMap& GetMaterials() const;
	SMaterial* FindMaterial(const string& Name) const;
	SMaterial* FindMaterial(uint32_t Index) const;
	uint32_t GetMaterialCBIndex(const string& Name);
	uint32_t GetNumMaterials();

	void LoadMaterialsFromCache();
	static void LoadTexturesFromPaths(vector<STexturePath>& OutTextures);
	static vector<STexturePath> LoadTexturesFromPaths(const vector<wstring>& Paths);
	static bool LoadTextureFromPath(const wstring& Path, STexturePath& OutTexture);
	void SaveMaterials() const;
	void BuildMaterialsFromTextures(const std::unordered_map<string, unique_ptr<STexture>>& Textures);
	OnMaterialsChanged MaterialsRebuld;
	void OnMaterialChanged(const string& Name);

private:
	SMutex MaterialsLock;
	TMaterialsMap Materials;
	TMaterialsIndexMap MaterialsIndicesMap;
	unique_ptr<OMaterialsConfigParser> MaterialsConfigParser;
};
