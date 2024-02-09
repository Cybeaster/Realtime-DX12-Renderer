#pragma once
#include "../GeomertryGenerator/GeometryGenerator.h"
#include "../MeshParser.h"
#include "DXHelper.h"

enum class EParserType
{
	Custom
};

class OMeshGenerator
{
public:
	unique_ptr<SMeshGeometry> CreateMesh(string Name, const OGeometryGenerator::SMeshData& Data, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	unique_ptr<SMeshGeometry> CreateMesh(const string& Name, const string& Path, EParserType Parser, ETextureMapType GenTexels, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
};
