#pragma once
#include "../DirectX/MaterialData.h"
#include "../Material.h"
#include "Types.h"

#include <unordered_map>

struct STexture;
class OMaterialManager
{
public:
	using TMaterialsMap = std::unordered_map<string, unique_ptr<SMaterial>>;

	void AddMaterial(string Name, unique_ptr<SMaterial>& Material);
	void CreateMaterial(const string& Name, int32_t CBIndex, int32_t DiffuseSRVHeapIdx, const SMaterialSurface& Surface);
	void CreateMaterial(const string& Name, STexture* Texture, const SMaterialSurface& Surface);
	const TMaterialsMap& GetMaterials() const;
	SMaterial* FindMaterial(const string& Name) const;
	uint32_t GetMaterialCBIndex(const string& Name) const;
	uint32_t GetNumMaterials() const
	{
		return Materials.size();
	}
	void BuildDefaultMaterials(std::unordered_map<string, unique_ptr<STexture>>& Textures);

private:
	TMaterialsMap Materials;
};
