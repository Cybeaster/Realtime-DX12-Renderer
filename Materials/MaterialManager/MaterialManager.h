#pragma once
#include "../../Config/MaterialsReader/MaterialsReader.h"
#include "../Material.h"
#include "DirectX/MaterialData.h"
#include "Types.h"

#include <unordered_map>

struct STexture;
class OMaterialManager
{
public:
	OMaterialManager();
	using TMaterialsMap = std::unordered_map<string, unique_ptr<SMaterial>>;

	void AddMaterial(string Name, unique_ptr<SMaterial>& Material);
	void CreateMaterial(const string& Name, int32_t CBIndex, int32_t DiffuseSRVHeapIdx, const SMaterialSurface& Surface);
	void CreateMaterial(const string& Name, STexture* Texture, const SMaterialSurface& Surface);

	const TMaterialsMap& GetMaterials() const;
	SMaterial* FindMaterial(const string& Name) const;

	uint32_t GetMaterialCBIndex(const string& Name) const;
	uint32_t GetNumMaterials() const;

	void LoadMaterials();
	void SaveMaterials() const;
	void BuildMaterialsFromTextures(const std::unordered_map<string, unique_ptr<STexture>>& Textures);

private:
	TMaterialsMap Materials;
	unique_ptr<OMaterialsConfigParser> MaterialsConfigParser;
};
