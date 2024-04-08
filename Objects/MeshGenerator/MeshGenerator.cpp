
#include "MeshGenerator.h"

#include "CommandQueue/CommandQueue.h"
#include "DirectX/Vertex.h"
#include "EngineHelper.h"
#include "Logger.h"
#include "MeshPayload.h"
#include "TinyObjLoader/TinyObjLoaderParser.h"
#include "tiny_obj_loader.h"
using namespace DirectX;
using namespace Utils::Math;
#pragma optimize("", off)

unique_ptr<SMeshGeometry> OMeshGenerator::CreateGridMesh(string Name, float Width, float Depth, uint32_t Row, uint32_t Column)
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

unique_ptr<SMeshGeometry> OMeshGenerator::CreateMesh(const SMeshPayloadData& Data) const
{
	std::vector<SVertex> vertices;
	std::vector<uint32_t> indices;
	size_t vertCounter = 0;
	size_t indexCounter = 0;
	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = Data.Name;
	const auto min = Data.MinCooridnate;
	const auto max = Data.MaxCooridnate;
	for (const auto& payload : Data.Data)
	{
		XMFLOAT3 vMinf3(+Infinity, +Infinity, +Infinity);
		XMFLOAT3 vMaxf3(-Infinity, -Infinity, -Infinity);

		XMVECTOR vMin = XMLoadFloat3(&vMinf3);
		XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
		std::vector<XMFLOAT3> positions(payload.Vertices.size());

		for (size_t i = 0; i < payload.Vertices.size(); ++i)
		{
			SVertex vertex;
			vertex.Position = payload.Vertices[i].Position;
			vertex.Normal = payload.Vertices[i].Normal;
			vertex.TexC = payload.Vertices[i].TexC;
			vertex.TangentU = payload.Vertices[i].TangentU;
			vertices.push_back(vertex);

			auto pos = XMLoadFloat3(&vertex.Position);
			positions[i] = vertex.Position;
			vMax = XMVectorMax(vMax, pos);
			vMin = XMVectorMin(vMin, pos);
		}

		BoundingBox bounds;
		XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
		XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

		SSubmeshGeometry submesh;
		submesh.Bounds = bounds;
		submesh.Vertices = make_unique<vector<XMFLOAT3>>(std::move(positions));
		submesh.Indices = make_unique<vector<std::uint32_t>>(payload.Indices32);
		submesh.IndexCount = payload.Indices32.size();
		submesh.StartIndexLocation = vertCounter;
		submesh.BaseVertexLocation = indexCounter;
		submesh.Name = payload.Name;
		submesh.Material = CreateMaterial(payload.Material);
		vertCounter += payload.Vertices.size();
		indexCounter += payload.Indices32.size();
		indices.insert(indices.end(), payload.Indices32.begin(), payload.Indices32.end());
		geo->SetGeometry(payload.Name, submesh);
	}

	UINT vbByteSize = vertices.size() * sizeof(SVertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::uint32_t);

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
	return move(geo);
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateMesh(const string& Name, const OGeometryGenerator::SMeshData& Data) const
{
	SMeshPayloadData payload = {
		.Data = { Data },
		.Name = Name,
		.TotalVertices = Data.Vertices.size(),
		.TotalIndices = Data.Indices32.size()
	};
	payload.Data[0].Name = Name;
	return CreateMesh(payload);
}

unique_ptr<SMeshGeometry> OMeshGenerator::CreateMesh(const string& Name, const wstring& Path, const EParserType Parser, ETextureMapType GenTexels)
{
	unique_ptr<IMeshParser> parser = nullptr;
	switch (Parser)
	{
	case EParserType::Custom:
		parser = IMeshParser::CreateParser<OCustomParser>();
		break;
	case EParserType::TinyObjLoader:
		parser = IMeshParser::CreateParser<OTinyObjParser>();
	}
	SMeshPayloadData meshData;
	meshData.Name = Name;
	const bool successful = parser->ParseMesh(Path, meshData, GenTexels);
	CWIN_LOG(!successful, Geometry, Error, "Failed to parse the mesh: {}", Path);
	if (successful)
	{
		return CreateMesh(meshData);
	}
	else
	{
		return nullptr;
	}
}

#pragma optimize("", on)