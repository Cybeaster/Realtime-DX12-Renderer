#include "MeshParser.h"

#include "../Utils/DirectXUtils.h"
#include "../Utils/Math.h"
#include "Logger.h"
#include "Vertex.h"

using namespace Utils::Math;
using namespace DirectX;
bool OCustomParser::ParseMesh(const string& Path, OGeometryGenerator::SMeshData& MeshData, ETextureMapType Type)
{
	std::ifstream fin(Path);
	using namespace Utils::Math;
	CWIN_LOG(!fin, Geometry, Warning, "Could not open the file: {}", TEXT(Path));
	if (!fin)
	{
		return false;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<OGeometryGenerator::SGeometryExtendedVertex> vertices(vcount);

	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Position.x >> vertices[i].Position.y >> vertices[i].Position.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		switch (Type)
		{
		case ETextureMapType::Spherical:
			const XMVECTOR P = XMLoadFloat3(&vertices[i].Position);
			// Project point onto unit sphere and generate spherical texture coordinates.
			XMFLOAT3 spherePos;
			XMStoreFloat3(&spherePos, XMVector3Normalize(P));

			float theta = atan2f(spherePos.z, spherePos.x);
			if (theta < 0.0f)
				theta += XM_2PI;

			float phi = acosf(spherePos.y);

			float u = theta / (2.0f * XM_PI);
			float v = phi / XM_PI;
			vertices[i].TexC = { u, v };
			break;
		}
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<uint32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	MeshData.Indices32.assign(indices.begin(), indices.end());
	MeshData.Vertices.assign(vertices.begin(), vertices.end());
	return true;
}