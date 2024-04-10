#pragma once
#include "GeomertryGenerator/GeometryGenerator.h"
struct SMeshPayloadData
{
	vector<OGeometryGenerator::SMeshData> Data;
	string Name = "NONE";
	size_t TotalVertices = 0;
	size_t TotalIndices = 0;
};