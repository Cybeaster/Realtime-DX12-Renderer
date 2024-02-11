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

inline auto& CreateGridRenderItem(string Category, string Name, float Width, float Depth, float Row, uint32_t Column, size_t NumberOfInstances, const SMaterialDisplacementParams& Params, string Submesh)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();

	return engine->BuildRenderItemFromMesh(Category, generator->CreateGridMesh(Name, Width, Depth, Row, Column), NumberOfInstances, Params, Submesh);
}

inline auto& CreateBoxRenderItem(string Category, string Name, float Width, float Height, float Depth, uint32_t NumSubdivisions, size_t NumberOfInstances, const SMaterialDisplacementParams& Params, string Submesh)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateBoxMesh(Name, Width, Height, Depth, NumSubdivisions), NumberOfInstances, Params, Submesh);
}

inline auto& CreateSphereRenderItem(string Category, string Name, float Radius, uint32_t SliceCount, uint32_t StackCount, size_t NumberOfInstances, const SMaterialDisplacementParams& Params, string Submesh)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateSphereMesh(Name, Radius, SliceCount, StackCount), NumberOfInstances, Params, Submesh);
}

inline auto& CreateGeosphereRenderItem(string Category, string Name, float Radius, uint32_t NumSubdivisions, size_t NumberOfInstances, const SMaterialDisplacementParams& Params, string Submesh)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateGeosphereMesh(Name, Radius, NumSubdivisions), NumberOfInstances, Params, Submesh);
}

inline auto& CreateCylinderRenderItem(string Category, string Name, float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, size_t NumberOfInstances, const SMaterialDisplacementParams& Params, string Submesh)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateCylinderMesh(Name, BottomRadius, TopRadius, Height, SliceCount, StackCount), NumberOfInstances, Params, Submesh);
}

inline auto& CreateQuadRenderItem(string Category, string Name, float X, float Y, float Width, float Height, float Depth, size_t NumberOfInstances, const SMaterialDisplacementParams& Params, string Submesh)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(Category, generator->CreateQuadMesh(Name, X, Y, Width, Height, Depth), NumberOfInstances, Params, Submesh);
}
