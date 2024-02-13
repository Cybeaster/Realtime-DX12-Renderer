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
	using TTexturesMapPath = std::unordered_map<wstring, STexture*>;
	OTextureManager(ID3D12Device* Device, OCommandQueue* CommandList);

	STexture* CreateTexture(string Name, wstring FileName);
	STexture* CreateTexture(const wstring& FileName);

	STexture* FindTextureByName(string Name) const;
	STexture* FindTextureByPath(wstring Path) const;
	STexture* FindOrCreateTexture(wstring FileName);
	STexture* FindOrCreateTexture(string Name, wstring FileName);

	TTexturesMap& GetTextures() { return Textures; }
	uint32_t GetNumTextures() const { return Textures.size(); }

private:
	void LoadLocalTextures();
	void AddTexture(unique_ptr<STexture> Texture);
	void RemoveTexture(const string& Name);
	void RemoveTexture(const wstring& Path);

	ID3D12Device* Device;
	OCommandQueue* CommandQueue;
	TTexturesMap Textures;
	TTexturesMapPath TexturesPath;
};
