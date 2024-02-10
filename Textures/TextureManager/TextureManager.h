#pragma once
#include "../Texture.h"
#include "Types.h"

#include <unordered_map>

class OCommandQueue;
class OEngine;
class OTextureManager
{
public:
	using TTexturesMap = std::unordered_map<string, unique_ptr<STexture>>;

	OTextureManager(ID3D12Device* Device, OCommandQueue* CommandList);

	STexture* CreateTexture(string Name, wstring FileName);
	STexture* FindTexture(string Name) const;
	TTexturesMap& GetTextures() { return Textures; }
	uint32_t GetNumTextures() const { return Textures.size(); }

private:
	void LoadLocalTextures();

	ID3D12Device* Device;
	OCommandQueue* CommandQueue;
	TTexturesMap Textures;
};
