#pragma once
#include "GeomertryGenerator/GeometryGenerator.h"
struct SMeshPayloadData
{
	vector<OGeometryGenerator::SMeshData> Data;
	string Name;
	size_t TotalVertices;
	size_t TotalIndices;
};