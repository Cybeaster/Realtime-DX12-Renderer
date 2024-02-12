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

inline auto& CreateGridRenderItem(string Category, string Name, float Width, float Depth, uint32_t Row, uint32_t Column, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateGridMesh(Name, Width, Depth, Row, Column), Params);
}

inline auto& CreateBoxRenderItem(string Category, string Name, float Width, float Height, float Depth, uint32_t NumSubdivisions, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateBoxMesh(Name, Width, Height, Depth, NumSubdivisions), Params);
}

inline auto& CreateSphereRenderItem(string Category, string Name, float Radius, uint32_t SliceCount, uint32_t StackCount, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateSphereMesh(Name, Radius, SliceCount, StackCount), Params);
}

inline auto& CreateGeosphereRenderItem(string Category, string Name, float Radius, uint32_t NumSubdivisions, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateGeosphereMesh(Name, Radius, NumSubdivisions), Params);
}

inline auto& CreateCylinderRenderItem(string Category, string Name, float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateCylinderMesh(Name, BottomRadius, TopRadius, Height, SliceCount, StackCount), Params);
}

inline auto& CreateQuadRenderItem(string Category, string Name, float X, float Y, float Width, float Height, float Depth, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateQuadMesh(Name, X, Y, Width, Height, Depth), Params);
}