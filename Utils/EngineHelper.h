#pragma once
#include "../Materials/MaterialManager/MaterialManager.h"
#include "../Types/TextureConstants.h"
#include "Engine/Engine.h"

inline void CreateMaterial(const string& Name, STexture* Texture, const SMaterialSurface& Constants)
{
	OEngine::Get()->GetMaterialManager()->CreateMaterial(Name, Texture, Constants);
}

inline auto& GetMaterials()
{
	return OEngine::Get()->GetMaterialManager()->GetMaterials();
}

inline SMaterial* FindMaterial(const string& Name)
{
	return OEngine::Get()->GetMaterialManager()->FindMaterial(Name);
}

inline void CreateTexture(const string& Name, wstring FileName)
{
	OEngine::Get()->GetTextureManager()->CreateTexture(Name, FileName);
}

inline STexture* FindTexture(const string& Name)
{
	return OEngine::Get()->GetTextureManager()->FindTexture(Name);
}