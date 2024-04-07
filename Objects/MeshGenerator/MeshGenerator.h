#pragma once
#include "../GeomertryGenerator/GeometryGenerator.h"
#include "../MeshParser.h"
#include "DirectX/DXHelper.h"

struct SMeshPayloadData;
class OCommandQueue;
enum class EParserType
{
	Custom,
	TinyObjLoader
};

class OMeshGenerator
{
public:
	OMeshGenerator(ID3D12Device* Device, OCommandQueue* CommandList)
	    : Device(Device)
	    , CommandQueue(CommandList)
	{
	}

	unique_ptr<SMeshGeometry> CreateGridMesh(string Name, float Width, float Height, uint32_t Row, uint32_t NumSubdivisions);
	unique_ptr<SMeshGeometry> CreateBoxMesh(string Name, float Width, float Height, float Depth, uint32_t NumSubdivisions);
	unique_ptr<SMeshGeometry> CreateSphereMesh(string Name, float Radius, uint32_t SliceCount, uint32_t StackCount);
	unique_ptr<SMeshGeometry> CreateCylinderMesh(string Name, float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount);
	unique_ptr<SMeshGeometry> CreateGeosphereMesh(string Name, float Radius, uint32_t NumSubdivisions);
	unique_ptr<SMeshGeometry> CreateQuadMesh(string Name, float X, float Y, float Width, float Height, float Depth);

	unique_ptr<SMeshGeometry> CreateMesh(const SMeshPayloadData& Data) const;
	unique_ptr<SMeshGeometry> CreateMesh(string Name, const OGeometryGenerator::SMeshData& Data) const;
	unique_ptr<SMeshGeometry> CreateMesh(const string& Name, const wstring& Path, EParserType Parser, ETextureMapType GenTexels);

private:
	OGeometryGenerator Generator;
	ID3D12Device* Device;
	OCommandQueue* CommandQueue;
};
