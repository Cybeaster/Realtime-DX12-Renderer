#pragma once
#include "MeshParser.h"
class OTinyObjParser : public IMeshParser
{
	bool ParseMesh(const wstring& Path, SMeshPayloadData& MeshData, ETextureMapType Type = ETextureMapType::None) override;
};
