#pragma once
#include "../Materials/MaterialManager/MaterialManager.h"
#include "../Types/TextureConstants.h"
#include "Application.h"
#include "DirectX/RenderItem/RenderItem.h"
#include "Engine/Engine.h"
#include "GraphicsPipeline/GraphicsPipeline.h"

inline void CreateMaterial(const string& Name, STexture* Texture, const HLSL::MaterialData& Constants, bool Notify = false)
{
	OEngine::Get()->GetMaterialManager()->CreateMaterial(Name, Texture, Constants, Notify);
}

inline SMaterial* CreateMaterial(const SMaterialPayloadData& Data)
{
	return OEngine::Get()->GetMaterialManager()->CreateMaterial(Data);
}

inline auto& GetMaterials()
{
	return OEngine::Get()->GetMaterialManager()->GetMaterials();
}

inline SMaterial* FindMaterial(const string& Name)
{
	return OEngine::Get()->GetMaterialManager()->FindMaterial(Name);
}

inline SMaterial* FindMaterial(int32_t Index)
{
	return OEngine::Get()->GetMaterialManager()->FindMaterial(Index);
}

inline void CreateTexture(const string& Name, wstring FileName)
{
	OEngine::Get()->GetTextureManager()->CreateTexture(Name, FileName);
}

inline STexture* FindTextureByName(const string& Name)
{
	return OEngine::Get()->GetTextureManager()->FindTextureByName(Name);
}

inline STexture* FindTextureByPath(const wstring& FileName)
{
	return OEngine::Get()->GetTextureManager()->FindTextureByPath(FileName);
}

inline STexture* FindOrCreateTexture(const string& Name, const wstring& FileName)
{
	return OEngine::Get()->GetTextureManager()->FindOrCreateTexture(Name, FileName);
}

inline STexture* FindOrCreateTexture(const wstring& FileName)
{
	return OEngine::Get()->GetTextureManager()->FindOrCreateTexture(FileName);
}

inline auto CreateRenderItem(const string& MeshName, const wstring& Path, EParserType ParserType, ETextureMapType TexelGenerator, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateMesh(MeshName, Path, ParserType, TexelGenerator), Params);
}

inline auto CreateRenderItem(const string& MeshName, const string& Path, EParserType ParserType, ETextureMapType TexelGenerator, const SRenderItemParams& Params)
{
	return CreateRenderItem(MeshName, UTF8ToWString(Path), ParserType, TexelGenerator, Params);
}

inline auto CreateGridRenderItem(string Name, float Width, float Depth, uint32_t Row, uint32_t Column, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateGridMesh(Name, Width, Depth, Row, Column), Params);
}

inline auto CreateBoxRenderItem(string Name, float Width, float Height, float Depth, uint32_t NumSubdivisions, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateBoxMesh(Name, Width, Height, Depth, NumSubdivisions), Params);
}

inline auto CreateSphereRenderItem(string Name, float Radius, uint32_t SliceCount, uint32_t StackCount, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateSphereMesh(Name, Radius, SliceCount, StackCount), Params);
}

inline auto CreateSphereRenderItem(string Name, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	auto& constants = SRenderConstants::Sphere;
	return engine->BuildRenderItemFromMesh(generator->CreateSphereMesh(Name, constants.Radius, constants.SliceCount, constants.StackCount), Params);
}

inline auto CreateGeosphereRenderItem(string Name, float Radius, uint32_t NumSubdivisions, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateGeosphereMesh(Name, Radius, NumSubdivisions), Params);
}

inline auto CreateCylinderRenderItem(string Name, float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateCylinderMesh(Name, BottomRadius, TopRadius, Height, SliceCount, StackCount), Params);
}

inline auto CreateQuadRenderItem(string Name, float X, float Y, float Width, float Height, float Depth, const SRenderItemParams& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	return engine->BuildRenderItemFromMesh(generator->CreateQuadMesh(Name, X, Y, Width, Height, Depth), Params);
}

inline auto GetSkyTextureSRV()
{
	return OEngine::Get()->GetSRVDescHandleForTexture(FindTextureByName(SRenderConstants::DefaultSkyTexture));
}

inline auto CreateLightSource(const SSpotLightPayload& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	auto mesh = generator->CreateSphereMesh("Spot Light Source", 0.5f, 20, 20);
	SRenderItemParams params;
	params.MaterialParams.Material = FindMaterial("White");
	auto ri = engine->BuildRenderItemFromMesh(std::move(mesh), params);
	auto component = engine->AddSpotLightComponent(ri);
	component->SetSpotLight(Params);
	return ri;
}

inline auto CreateLightSource(const SDirectionalLightPayload& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	auto mesh = generator->CreateSphereMesh("Directional Light Source", 0.5f, 20, 20);
	SRenderItemParams params;
	params.MaterialParams.Material = FindMaterial("White");
	auto ri = engine->BuildRenderItemFromMesh(std::move(mesh), params);
	auto component = engine->AddDirectionalLightComponent(ri);
	component->SetDirectionalLight(Params);
	return ri;
}

inline auto CreateLightSource(const string& Category, const SPointLightPayload& Params)
{
	auto engine = OEngine::Get();
	auto generator = engine->GetMeshGenerator();
	auto mesh = generator->CreateSphereMesh("Point Light Source", 0.5f, 20, 20);
	SRenderItemParams params;
	params.MaterialParams.Material = FindMaterial("White");
	auto ri = engine->BuildRenderItemFromMesh(std::move(mesh), params);
	auto component = engine->AddPointLightComponent(ri);
	component->SetPointLight(Params);
	return ri;
}

inline auto GetResourcePath(const wstring& Path)
{
	return OApplication::Get()->GetResourcePath(Path);
}