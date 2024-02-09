#include "GeometryGenerator.h"

#include "../../Utils/Path.h"
#include "Logger.h"

#include <fstream>

using namespace DirectX;
using namespace Utils::Math;
OGeometryGenerator::SMeshData OGeometryGenerator::CreateBox(float Width, float Height, float Depth, uint32_t NumSubdivisions)
{
	SMeshData data;
	SGeometryExtendedVertex v[24];

	const float w2 = 0.5f * Width;
	const float h2 = 0.5f * Height;
	const float d2 = 0.5f * Depth;

	//clang-format off

	// Fill in the front face vertex data.
	v[0] = SGeometryExtendedVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = SGeometryExtendedVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = SGeometryExtendedVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = SGeometryExtendedVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = SGeometryExtendedVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = SGeometryExtendedVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = SGeometryExtendedVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = SGeometryExtendedVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = SGeometryExtendedVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = SGeometryExtendedVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = SGeometryExtendedVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = SGeometryExtendedVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = SGeometryExtendedVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = SGeometryExtendedVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = SGeometryExtendedVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = SGeometryExtendedVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = SGeometryExtendedVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = SGeometryExtendedVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = SGeometryExtendedVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = SGeometryExtendedVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = SGeometryExtendedVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = SGeometryExtendedVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = SGeometryExtendedVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = SGeometryExtendedVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	data.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32_t i[36];

	// Fill in the front face index data
	i[0] = 0;
	i[1] = 1;
	i[2] = 2;
	i[3] = 0;
	i[4] = 2;
	i[5] = 3;

	// Fill in the back face index data
	i[6] = 4;
	i[7] = 5;
	i[8] = 6;
	i[9] = 4;
	i[10] = 6;
	i[11] = 7;

	// Fill in the top face index data
	i[12] = 8;
	i[13] = 9;
	i[14] = 10;
	i[15] = 8;
	i[16] = 10;
	i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12;
	i[19] = 13;
	i[20] = 14;
	i[21] = 12;
	i[22] = 14;
	i[23] = 15;

	// Fill in the left face index data
	i[24] = 16;
	i[25] = 17;
	i[26] = 18;
	i[27] = 16;
	i[28] = 18;
	i[29] = 19;

	// Fill in the right face index data
	i[30] = 20;
	i[31] = 21;
	i[32] = 22;
	i[33] = 20;
	i[34] = 22;
	i[35] = 23;

	//clang-format on

	data.Indices32.assign(&i[0], &i[36]);

	// Put a cap on the number of subdivisions.
	NumSubdivisions = std::min<uint32_t>(NumSubdivisions, 6u);

	for (uint32_t i = 0; i < NumSubdivisions; ++i)
		Subdivide(data);

	return data;
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateSphere(float Radius, uint32_t SliceCount, uint32_t StackCount)
{
	SMeshData meshData;
	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	SGeometryExtendedVertex topVertex(0.0f, +Radius, 0.0f, 0.0f, 1.0f, 0.0f, 1.0, 0.0f, 0.0f, 1.0f, 0.0f);
	SGeometryExtendedVertex bottomVertex(0.0f, -Radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f);

	meshData.Vertices.push_back(topVertex);

	const float phiStep = XM_PI / StackCount;
	const float thetaStep = 2.0f * XM_PI / SliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32_t i = 1; i <= StackCount - 1; ++i)
	{
		const float phi = i * phiStep;
		for (uint32_t j = 0; j <= SliceCount; ++j)
		{
			const float theta = j * thetaStep;

			SGeometryExtendedVertex vertex;

			vertex.Position.x = Radius * sinf(phi) * cosf(theta);
			vertex.Position.y = Radius * cosf(phi);
			vertex.Position.z = Radius * sinf(phi) * sinf(theta);

			vertex.TangentU.x = -Radius * sinf(phi) * sinf(theta);
			vertex.TangentU.y = 0.0f;
			vertex.TangentU.z = Radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMStoreFloat3(&vertex.TangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&vertex.Position);
			XMStoreFloat3(&vertex.Normal, XMVector3Normalize(p));

			vertex.TexC.x = theta / XM_2PI;
			vertex.TexC.y = phi / XM_PI;

			meshData.Vertices.push_back(std::move(vertex));
		}
	}

	meshData.Vertices.push_back(std::move(bottomVertex));

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32_t i = 1; i <= SliceCount; ++i)
	{
		meshData.Indices32.push_back(0);
		meshData.Indices32.push_back(i + 1);
		meshData.Indices32.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.

	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = SliceCount + 1;

	for (uint32_t i = 0; i < StackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < SliceCount; ++j)
		{
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// South pole vertex was added last.
	uint32_t southPoleIndex = static_cast<uint32_t>(meshData.Vertices.size()) - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32_t i = 0; i < SliceCount; ++i)
	{
		meshData.Indices32.push_back(southPoleIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}

	return meshData;
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateGeosphere(float Radius, uint32_t NumSubdivisions)
{
	SMeshData meshData;

	NumSubdivisions = std::min(NumSubdivisions, 6u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f;
	const float Z = 0.850651f;

	//clang-format off

	// Create 12 vertices of a icosahedron.
	const XMFLOAT3 positions[12] = {
		XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z), XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z), XMFLOAT3(0.0f, Z, X), XMFLOAT3(0.0f, Z, -X), XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X), XMFLOAT3(Z, X, 0.0f), XMFLOAT3(-Z, X, 0.0f), XMFLOAT3(Z, -X, 0.0f), XMFLOAT3(-Z, -X, 0.0f)
	};

	uint32_t k[60] = {
		1, 4, 0, 4, 9, 0, 4, 5, 9, 8, 5, 4, 1, 8, 4, 1, 10, 8, 10, 3, 8, 8, 3, 5, 3, 2, 5, 3, 7, 2, 3, 10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6, 1, 0, 10, 1, 6, 11, 0, 9, 2, 11, 9, 5, 2, 9, 11, 2, 7
	};
	//clang-format on

	meshData.Vertices.resize(12);
	meshData.Indices32.assign(&k[0], &k[60]);

	for (uint32_t i = 0; i < 12; ++i)
		meshData.Vertices[i].Position = positions[i];

	for (uint32_t i = 0; i < NumSubdivisions; ++i)
		Subdivide(meshData);

	for (uint32_t i = 0; i < meshData.Vertices.size(); ++i)
	{
		//Project vertex onto unit sphere and scale by radius.
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.Vertices[i].Position));

		// Project onto sphere.
		XMVECTOR p = Radius * n;

		XMStoreFloat3(&meshData.Vertices[i].Position, p);
		XMStoreFloat3(&meshData.Vertices[i].Normal, n);

		// Derive texture coordinates from spherical coordinates.
		float theta = atan2f(meshData.Vertices[i].Position.z, meshData.Vertices[i].Position.x);

		//Put in [0, 2pi].
		if (theta < 0.0f)
			theta += XM_2PI;

		const float phi = acosf(meshData.Vertices[i].Position.y / Radius);

		meshData.Vertices[i].TexC.x = theta / XM_2PI;
		meshData.Vertices[i].TexC.y = phi / XM_PI;

		//Partial derivative of P with respect to theta.
		meshData.Vertices[i].TangentU.x = -Radius * sinf(phi) * sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +Radius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
		XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));
	}
	return meshData;
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateCylinder(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount)
{
	SMeshData meshData;

	// Build vertices
	//------------------------------------------
	const float stackHeight = Height / StackCount;
	const float radiusStep = (TopRadius - BottomRadius) / StackCount;
	const uint32_t ringCount = StackCount + 1;

	for (uint32_t i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * Height + i * stackHeight;
		float r = BottomRadius + i * radiusStep;

		// vertices of ring
		const float dTheta = 2.0f * XM_PI / SliceCount;
		for (uint32_t j = 0; j <= SliceCount; j++)
		{
			SGeometryExtendedVertex vertex;

			const float c = cosf(j * dTheta);
			const float s = sinf(j * dTheta);

			vertex.Position = XMFLOAT3(r * c, y, r * s);
			vertex.TexC.x = static_cast<float>(j) / SliceCount;
			vertex.TexC.y = 1.0f - static_cast<float>(i) / StackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the
			// v tex-coord.
			// Let r0 be the bottom radius and let r1 be the top radius.
			// y(v) = h - hv for v in [0,1].
			// r(v) = r1 + (r0-r1)v
			//
			// x(t, v) = r(v)*cos(t)
			// y(t, v) = h - hv
			// z(t, v) = r(v)*sin(t)
			//
			// dx/dt = -r(v)*sin(t)
			// dy/dt = 0
			// dz/dt = +r(v)*cos(t)
			//
			// dx/dv = (r0-r1)*cos(t)
			// dy/dv = -h
			// dz/dv = (r0-r1)*sin(t)

			vertex.TangentU = XMFLOAT3(-s, 0.0f, c);
			float dr = BottomRadius - TopRadius;
			XMFLOAT3 bitangent(dr * c, -Height, dr * s);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.Normal, N);
			meshData.Vertices.push_back(std::move(vertex));
		}
	}
	//------------------------------------------

	// build indices

	const uint32_t ringVertexCount = SliceCount + 1;

	for (uint32_t i = 0; i < StackCount; ++i)
	{
		for (uint32_t j = 0; j < SliceCount; ++j)
		{
			meshData.Indices32.push_back(i * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.Indices32.push_back(i * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.Indices32.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(BottomRadius, TopRadius, Height, SliceCount, StackCount, meshData);
	BuildCylinderBottomCap(BottomRadius, TopRadius, Height, SliceCount, StackCount, meshData);
	return meshData;
}

void OGeometryGenerator::BuildCylinderTopCap(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, SMeshData& MeshData)
{
	const uint32_t baseIndex = static_cast<uint32_t>(MeshData.Vertices.size());

	float y = 0.5f * Height;
	const float dTheta = 2.0f * XM_PI / SliceCount;

	for (uint32_t i = 0; i <= SliceCount; ++i)
	{
		float x = TopRadius * cosf(i * dTheta);
		float z = TopRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		//  proportional to base.
		float u = x / Height + 0.5f;
		float v = z / Height + 0.5f;

		MeshData.Vertices.push_back({ x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v });
	}

	// Cap center vertex.
	MeshData.Vertices.push_back({ 0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f });

	const uint32_t centerIndex = static_cast<uint32_t>(MeshData.Vertices.size()) - 1;

	for (uint32_t i = 0; i < SliceCount; ++i)
	{
		MeshData.Indices32.push_back(centerIndex);
		MeshData.Indices32.push_back(baseIndex + i + 1);
		MeshData.Indices32.push_back(baseIndex + i);
	}
}

void OGeometryGenerator::BuildCylinderBottomCap(float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount, SMeshData& meshData)
{
	//
	// Build bottom cap.
	//

	const uint32_t baseIndex = static_cast<uint32_t>(meshData.Vertices.size());
	float y = -0.5f * Height;

	// vertices of ring
	const float dTheta = 2.0f * XM_PI / SliceCount;
	for (uint32_t i = 0; i <= SliceCount; ++i)
	{
		const float x = BottomRadius * cosf(i * dTheta);
		const float z = BottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		const float u = x / Height + 0.5f;
		const float v = z / Height + 0.5f;

		meshData.Vertices.push_back(SGeometryExtendedVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(SGeometryExtendedVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	const uint32_t centerIndex = static_cast<uint32_t>(meshData.Vertices.size()) - 1;

	for (uint32_t i = 0; i < SliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}
}

void OGeometryGenerator::Subdivide(SMeshData& MeshData)
{
	// Save a copy of the input geometry.
	SMeshData inputCopy = MeshData;

	MeshData.Vertices.resize(0);
	MeshData.Indices32.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	uint32_t numTris = (uint32_t)inputCopy.Indices32.size() / 3;
	for (uint32_t i = 0; i < numTris; ++i)
	{
		SGeometryExtendedVertex v0 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 0]];
		SGeometryExtendedVertex v1 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 1]];
		SGeometryExtendedVertex v2 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		SGeometryExtendedVertex m0 = MidPoint(v0, v1);
		SGeometryExtendedVertex m1 = MidPoint(v1, v2);
		SGeometryExtendedVertex m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		MeshData.Vertices.push_back(v0); // 0
		MeshData.Vertices.push_back(v1); // 1
		MeshData.Vertices.push_back(v2); // 2
		MeshData.Vertices.push_back(m0); // 3
		MeshData.Vertices.push_back(m1); // 4
		MeshData.Vertices.push_back(m2); // 5

		MeshData.Indices32.push_back(i * 6 + 0);
		MeshData.Indices32.push_back(i * 6 + 3);
		MeshData.Indices32.push_back(i * 6 + 5);

		MeshData.Indices32.push_back(i * 6 + 3);
		MeshData.Indices32.push_back(i * 6 + 4);
		MeshData.Indices32.push_back(i * 6 + 5);

		MeshData.Indices32.push_back(i * 6 + 5);
		MeshData.Indices32.push_back(i * 6 + 4);
		MeshData.Indices32.push_back(i * 6 + 2);

		MeshData.Indices32.push_back(i * 6 + 3);
		MeshData.Indices32.push_back(i * 6 + 1);
		MeshData.Indices32.push_back(i * 6 + 4);
	}
}

OGeometryGenerator::SGeometryExtendedVertex OGeometryGenerator::MidPoint(const SGeometryExtendedVertex& V0, const SGeometryExtendedVertex& V1)
{
	XMVECTOR p0 = XMLoadFloat3(&V0.Position);
	XMVECTOR p1 = XMLoadFloat3(&V1.Position);

	XMVECTOR n0 = XMLoadFloat3(&V0.Normal);
	XMVECTOR n1 = XMLoadFloat3(&V1.Normal);

	XMVECTOR tan0 = XMLoadFloat3(&V0.TangentU);
	XMVECTOR tan1 = XMLoadFloat3(&V1.TangentU);

	XMVECTOR tex0 = XMLoadFloat2(&V0.TexC);
	XMVECTOR tex1 = XMLoadFloat2(&V1.TexC);

	// Compute the midpoints of all the attributes.  Vectors need to be normalized
	// since linear interpolating can make them not unit length.
	XMVECTOR pos = 0.5f * (p0 + p1);
	XMVECTOR normal = XMVector3Normalize(0.5f * (n0 + n1));
	XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
	XMVECTOR tex = 0.5f * (tex0 + tex1);

	SGeometryExtendedVertex v;
	XMStoreFloat3(&v.Position, pos);
	XMStoreFloat3(&v.Normal, normal);
	XMStoreFloat3(&v.TangentU, tangent);
	XMStoreFloat2(&v.TexC, tex);

	return v;
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateGrid(float Width, float Depth, uint32_t M, uint32_t N)
{
	SMeshData meshData;

	uint32_t vertexCount = M * N;
	uint32_t faceCount = (M - 1) * (N - 1) * 2;

	float halfWidth = 0.5f * Width;
	float halfDepth = 0.5f * Depth;

	float dx = Width / (N - 1);
	float dz = Depth / (M - 1);

	float du = 1.0f / (N - 1);
	float dv = 1.0f / (M - 1);

	meshData.Vertices.resize(vertexCount);

	for (uint32_t i = 0; i < M; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32_t j = 0; j < N; j++)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i * N + j].Position = XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[i * N + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * N + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i * N + j].TexC.x = j * du;
			meshData.Vertices[i * N + j].TexC.y = i * dv;
		}
	}

	// Create the indices.
	meshData.Indices32.resize(faceCount * 3); // 3 indices per face

	uint32_t k = 0;
	for (uint32_t i = 0; i < M - 1; i++)
	{
		for (uint32_t j = 0; j < N - 1; j++)
		{
			meshData.Indices32[k] = i * N + j;
			meshData.Indices32[k + 1] = i * N + j + 1;
			meshData.Indices32[k + 2] = (i + 1) * N + j;

			meshData.Indices32[k + 3] = (i + 1) * N + j;
			meshData.Indices32[k + 4] = i * N + j + 1;
			meshData.Indices32[k + 5] = (i + 1) * N + j + 1;
			k += 6;
		}
	}

	return meshData;
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateQuad(float X, float Y, float Width, float Height, float Depth)
{
	SMeshData meshData;

	meshData.Vertices.resize(4);
	meshData.Indices32.resize(6);

	// clang-format off

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = SGeometryExtendedVertex(
		X, Y - Height, Depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = SGeometryExtendedVertex(
		X, Y, Depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = SGeometryExtendedVertex(
		X+Width, Y, Depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = SGeometryExtendedVertex(
		X+Width, Y-Height, Depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	// clang-format on

	meshData.Indices32[0] = 0;
	meshData.Indices32[1] = 1;
	meshData.Indices32[2] = 2;
	meshData.Indices32[3] = 0;
	meshData.Indices32[4] = 2;
	meshData.Indices32[5] = 3;

	return meshData;
}

unique_ptr<SMeshGeometry> OGeometryGenerator::CreateSkullGeometry(string PathToModel, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, OEngine* Engine)
{
	std::ifstream fin(PathToModel);
	using namespace Utils::Math;
	CWIN_LOG(!fin, Geometry, Warning, "Could not open the file: {}", TO_STRING(PathToModel));

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+Infinity, +Infinity, +Infinity);
	XMFLOAT3 vMaxf3(-Infinity, -Infinity, -Infinity);
	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	std::vector<SVertex> vertices(vcount);
	std::vector<XMFLOAT3> positions(vcount);

	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Position.x >> vertices[i].Position.y >> vertices[i].Position.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Position);
		positions[i] = vertices[i].Position;
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

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<uint32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(int32_t);

	auto geometry = std::make_unique<SMeshGeometry>();
	geometry->Name = "Skull";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geometry->VertexBufferCPU));
	CopyMemory(geometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geometry->IndexBufferCPU));
	CopyMemory(geometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->VertexBufferGPU = Utils::CreateDefaultBuffer(
	    Device,
	    CommandList,
	    vertices.data(),
	    vbByteSize,
	    geometry->VertexBufferUploader);

	geometry->IndexBufferGPU = Utils::CreateDefaultBuffer(
	    Device,
	    CommandList,
	    indices.data(),
	    ibByteSize,
	    geometry->IndexBufferUploader);

	geometry->VertexByteStride = sizeof(SVertex);
	geometry->VertexBufferByteSize = vbByteSize;
	geometry->IndexFormat = DXGI_FORMAT_R32_UINT;
	geometry->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.Bounds = bounds;

	submesh.Vertices = make_unique<vector<XMFLOAT3>>(std::move(positions));
	submesh.Indices = make_unique<vector<uint32_t>>(std::move(indices));

	geometry->SetGeometry(geometry->Name, submesh);
	return std::move(geometry);
}

unique_ptr<SMeshGeometry> OGeometryGenerator::CreateWaterGeometry(float Width, float Depth, uint32_t RowCount, uint32_t ColumnCount, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, size_t VertexCount)
{
	SMeshData grid = CreateGrid(Width, Depth, RowCount, ColumnCount);
	std::vector<SVertex> vertices(grid.Vertices.size());
	XMFLOAT3 vMinf3(+Infinity, +Infinity, +Infinity);
	XMFLOAT3 vMaxf3(-Infinity, -Infinity, -Infinity);
	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		vertices[i].Position = grid.Vertices[i].Position;
		vertices[i].Normal = grid.Vertices[i].Normal;
		vertices[i].TexC = grid.Vertices[i].TexC;
		auto pos = XMLoadFloat3(&vertices[i].Position);

		vMax = XMVectorMax(vMax, pos);
		vMin = XMVectorMin(vMin, pos);
	}
	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

	std::vector<std::uint32_t> indices = grid.Indices32;

	UINT vbByteSize = VertexCount * sizeof(SVertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = "WaterGeometry";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(Device,
	                                                  CommandList,
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(Device,
	                                                 CommandList,
	                                                 indices.data(),
	                                                 ibByteSize,
	                                                 geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.Bounds = bounds;

	geo->SetGeometry("Grid", submesh);
	return move(geo);
}
