#include "GeometryGenerator.h"

using namespace DirectX;

OGeometryGenerator::SMeshData OGeometryGenerator::CreateBox(float Width, float Height, float Depth, uint32_t NumSubdivisions)
{
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
	SVertex topVertex(0.0f, +Radius, 0.0f, 0.0f, 1.0f, 0.0f, 1.0, 0.0f, 0.0f, 1.0f, 0.0f);
	SVertex bottomVertex(0.0f, -Radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f);

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

			SVertex vertex;

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
	const XMFLOAT3 positions[12] =
	{
		XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X), XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f), XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f), XMFLOAT3(-Z, -X, 0.0f)
	};

	uint32_t k[60] =
	{
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7
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
			SVertex vertex;

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

		meshData.Vertices.push_back(SVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(SVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

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
	SMeshData copiedMeshData = MeshData;

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

	uint32_t numTris = static_cast<uint32_t>(copiedMeshData.Indices32.size()) / 3;

	for (uint32_t i = 0; i < numTris; i++)
	{
		SVertex v0 = copiedMeshData.Vertices[copiedMeshData.Indices32[i * 3 + 0]];
		SVertex v1 = copiedMeshData.Vertices[copiedMeshData.Indices32[i * 3 + 1]];
		SVertex v2 = copiedMeshData.Vertices[copiedMeshData.Indices32[i * 3 + 2]];

		SVertex m0 = MidPoint(v0, v1);
		SVertex m1 = MidPoint(v1, v2);
		SVertex m2 = MidPoint(v0, v2);

		//Add new geometry
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

OGeometryGenerator::SVertex OGeometryGenerator::MidPoint(const SVertex& V0, const SVertex& V1)
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

	SVertex v;
	XMStoreFloat3(&v.Position, pos);
	XMStoreFloat3(&v.Normal, normal);
	XMStoreFloat3(&v.TangentU, tangent);
	XMStoreFloat2(&v.TexC, tex);

	return v;
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateGrid(float Width, float Depth, uint32_t M, uint32_t N)
{
}

OGeometryGenerator::SMeshData OGeometryGenerator::CreateQuad(float X, float Y, float Width, float Height, float Depth)
{
}
