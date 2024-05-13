#pragma once
#include "DirectX/MeshGeometry.h"
#include "Material.h"

#include <DirectX/DXHelper.h>
#include <Types.h>

struct SMaterial;
enum class EGeometryType
{
	Grid,
	Box,
	Sphere,
	Cylinder,
	GeoSphere,
	Quad
};

class OEngine;
class OGeometryGenerator
{
public:
	struct SGeometryExtendedVertex;
	struct SMeshData;

	SMeshData CreateBox(float Width, float Height, float Depth, uint32_t NumSubdivisions);
	SMeshData CreateCube(float Width, float Height, float Depth, uint32_t NumSubdivisions);
	SMeshData CreateSphere(float Radius, uint32_t SliceCount, uint32_t StackCount);
	SMeshData CreateGeosphere(float Radius, uint32_t NumSubdivisions);
	SMeshData CreateCylinder(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount,
	                         uint32_t StackCount);
	SMeshData CreateGrid(float Width, float Depth, uint32_t M, uint32_t N);
	SMeshData CreateQuad(float X, float Y, float Width, float Height, float Depth);

	unique_ptr<SMeshGeometry> CreateSkullGeometry(string PathToModel, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, OEngine* Engine);
	unique_ptr<SMeshGeometry> CreateWaterGeometry(float Width, float Depth, uint32_t RowCount, uint32_t ColumnCount, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, size_t VertexCount);

private:
	void BuildCylinderTopCap(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, SMeshData& MeshData);
	void BuildCylinderBottomCap(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, SMeshData& meshData);
	void Subdivide(SMeshData& MeshData);
	SGeometryExtendedVertex MidPoint(const SGeometryExtendedVertex& V0, const SGeometryExtendedVertex& V1);
};

struct OGeometryGenerator::SGeometryExtendedVertex
{
	SGeometryExtendedVertex() = default;

	SGeometryExtendedVertex(float X, float Y, float Z,
	                        float NX, float NY, float NZ,
	                        float TX, float TY, float TZ,
	                        float U, float V)
	    : Position(X, Y, Z)
	    , Normal(NX, NY, NZ)
	    , TangentU(TX, TY, TZ)
	    , TexC(U, V)

	{
	}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
};

struct OGeometryGenerator::SMeshData
{
	vector<SGeometryExtendedVertex> Vertices;
	vector<uint32_t> Indices32;
	string Name;
	const vector<uint16_t>& GetIndices16()
	{
		if (Indices16.empty())
		{
			Indices16.resize(Indices32.size());
			for (size_t i = 0; i < Indices32.size(); ++i)
				Indices16[i] = static_cast<uint16_t>(Indices32[i]);
		}

		return Indices16;
	}

	SMaterialPayloadData Material;

private:
	vector<uint16_t> Indices16;
};
