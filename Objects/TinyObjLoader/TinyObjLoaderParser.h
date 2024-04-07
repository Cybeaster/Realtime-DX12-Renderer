#pragma once
#include "MeshParser.h"
class OTinyObjParser : public IMeshParser
{
	bool ParseMesh(const wstring& Path, OGeometryGenerator::SMeshData& MeshData, ETextureMapType Type = ETextureMapType::None) override;
};
