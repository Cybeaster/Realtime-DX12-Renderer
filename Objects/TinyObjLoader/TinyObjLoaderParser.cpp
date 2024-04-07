#include "TinyObjLoaderParser.h"

#include "Logger.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "MeshGenerator/MeshPayload.h"
#include "tiny_obj_loader.h"

bool OTinyObjParser::ParseMesh(const wstring& Path, SMeshPayloadData& MeshData, ETextureMapType Type)
{
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = ""; // Path to material files

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(WStringToUTF8(Path), reader_config))
	{
		if (!reader.Error().empty())
		{
			WIN_LOG(TinyObjLoader, Error, "TinyObjReader: {}", TEXT(reader.Error()));
		}
		return false;
	}

	if (!reader.Warning().empty())
	{
		LOG(TinyObjLoader, Warning, "TinyObjReader: {}", TEXT(reader.Warning()));
		return false;
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		OGeometryGenerator::SMeshData data;
		data.Name = shapes[s].name;
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			const size_t fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				OGeometryGenerator::SGeometryExtendedVertex vertex{};
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertex.Position.x = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
				vertex.Position.y = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1];
				vertex.Position.z = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2];

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0)
				{
					vertex.Normal.x = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0];
					vertex.Normal.y = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1];
					vertex.Normal.z = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0)
				{
					vertex.TexC.x = attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0];
					vertex.TexC.y = attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1];
				}

				// Optional: vertex colors
				// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
			}
			index_offset += fv;

			// per-face material
			//shapes[s].mesh.material_ids[f];
		}
		MeshData.Data.push_back(std::move(data));
		MeshData.TotalIndices += shapes[s].mesh.indices.size();
		MeshData.TotalVertices += static_cast<uint32_t>(attrib.vertices.size());
	}
	return true;
}