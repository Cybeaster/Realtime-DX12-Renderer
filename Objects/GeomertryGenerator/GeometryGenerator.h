#pragma once
#include <DirectX/DXHelper.h>
#include <Types.h>

class OGeometryGenerator
{
public:
	struct SVertex;
	struct SMeshData;

	SMeshData CreateBox(float Width, float Height, float Depth, uint32_t NumSubdivisions);

	SMeshData CreateSphere(float Radius, uint32_t SliceCount, uint32_t StackCount);

	SMeshData CreateGeosphere(float Radius, uint32_t NumSubdivisions);

	SMeshData CreateCylinder(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount,
	                         uint32_t StackCount);

	SMeshData CreateGrid(float Width, float Depth, uint32_t M, uint32_t N);

	SMeshData CreateQuad(float X, float Y, float Width, float Height, float Depth);

private:
	void BuildCylinderTopCap(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, SMeshData& MeshData);

	void BuildCylinderBottomCap(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, SMeshData& meshData);

	void Subdivide(SMeshData& MeshData);

	SVertex MidPoint(const SVertex& V0, const SVertex& V1);
};

struct OGeometryGenerator::SVertex
{
	SVertex() = default;

	SVertex(float X, float Y, float Z, float NX, float NY, float NZ, float U, float V, float TX, float TY, float TZ)
		: Position(X, Y, Z)
		, Normal(NX, NY, NZ)
		, TexC(U, V)
		, TangentU(TX, TY, TZ)
	{
	}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
};

struct OGeometryGenerator::SMeshData
{
	vector<SVertex> Vertices;
	vector<uint32_t> Indices32;

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

private:
	vector<uint16_t> Indices16;
};
