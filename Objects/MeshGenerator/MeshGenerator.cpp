//
// Created by Cybea on 08/02/2024.
//

#include "MeshGenerator.h"

#include "../../Utils/DirectX.h"
#include "../../Utils/Math.h"
#include "../MeshParser.h"
#include "CommandQueue/CommandQueue.h"
#include "Logger.h"
#include "Vertex.h"
using namespace DirectX;
using namespace Utils::Math;

unique_ptr<SMeshGeometry> OMeshGenerator::CreateGridMesh(string Name, float Width, float Depth, float Row, uint32_t Column)
{
	return CreateMesh(Name, Generator.CreateGrid(Width, Depth, Row, Column));
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateBoxMesh(string Name, float Width, float Height, float Depth, uint32_t NumSubdivisions)
{
	return CreateMesh(Name, Generator.CreateBox(Width, Height, Depth, NumSubdivisions));
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateSphereMesh(string Name, float Radius, uint32_t SliceCount, uint32_t StackCount)
{
	return CreateMesh(Name, Generator.CreateSphere(Radius, SliceCount, StackCount));
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateCylinderMesh(string Name, float BottomRadius, float TopRadius, float Height, uint32_t SliceCount, uint32_t StackCount)
{
	return CreateMesh(Name, Generator.CreateCylinder(BottomRadius, TopRadius, Height, SliceCount, StackCount));
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateGeosphereMesh(string Name, float Radius, uint32_t NumSubdivisions)
{
	return CreateMesh(Name, Generator.CreateGeosphere(Radius, NumSubdivisions));
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateQuadMesh(string Name, float X, float Y, float Width, float Height, float Depth)
{
	return CreateMesh(Name, Generator.CreateQuad(X, Y, Width, Height, Depth));
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateMesh(string Name, const OGeometryGenerator::SMeshData& Data) const
{
	std::vector<SVertex> vertices(Data.Vertices.size());

	XMFLOAT3 vMinf3(+Infinity, +Infinity, +Infinity);
	XMFLOAT3 vMaxf3(-Infinity, -Infinity, -Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	std::vector<XMFLOAT3> positions(Data.Vertices.size());

	for (size_t i = 0; i < Data.Vertices.size(); ++i)
	{
		vertices[i].Position = Data.Vertices[i].Position;
		vertices[i].Normal = Data.Vertices[i].Normal;
		vertices[i].TexC = Data.Vertices[i].TexC;
		auto pos = XMLoadFloat3(&vertices[i].Position);
		positions[i] = vertices[i].Position;
		vMax = XMVectorMax(vMax, pos);
		vMin = XMVectorMin(vMin, pos);
	}

	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

	std::vector<std::uint32_t> indices = Data.Indices32;

	UINT vbByteSize = vertices.size() * sizeof(SVertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::uint32_t);

	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = Name;

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(Device,
	                                                  CommandQueue->GetCommandList().Get(),
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(Device,
	                                                 CommandQueue->GetCommandList().Get(),
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
	submesh.Name = Name;
	submesh.Vertices = make_unique<vector<XMFLOAT3>>(std::move(positions));
	submesh.Indices = make_unique<vector<std::uint32_t>>(std::move(indices));
	geo->SetGeometry(Name, submesh);
	return move(geo);
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateMesh(const string& Name, const string& Path, const EParserType Parser, ETextureMapType GenTexels)
{
	unique_ptr<IMeshParser> parser = nullptr;
	switch (Parser)
	{
	case EParserType::Custom:
		parser = IMeshParser::CreateParser<OCustomParser>();
		break;
	}

	OGeometryGenerator::SMeshData data;
	const bool successful = parser->ParseMesh(Path, data, GenTexels);
	CWIN_LOG(!successful, Geometry, Warning, "Failed to parse the mesh: {}", TO_STRING(Path));
	if (successful)
	{
		return CreateMesh(Name, data);
	}
	else
	{
		return nullptr;
	}
}