#include "TinyObjLoaderParser.h"

#include "Logger.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "Application.h"
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
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();
	MeshData.Data.reserve(shapes.size());
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		OGeometryGenerator::SMeshData data;
		data.Name = shapes[s].name;
		data.Vertices.reserve(shapes[s].mesh.indices.size() / 3);
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
				tinyobj::TinyObjPoint point = { attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0],
					                            attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1],
					                            attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2] };
				vertex.Position = { point.x, point.y, point.z };
				auto min = std::min({ vertex.Position.x, vertex.Position.y, vertex.Position.z });
				auto max = std::max({ vertex.Position.x, vertex.Position.y, vertex.Position.z });
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

				data.Vertices.push_back(vertex);
				data.Indices32.push_back(static_cast<uint32_t>(data.Indices32.size()));
			}
			index_offset += fv;
		}

		// per-face material
		const auto id = shapes[s].mesh.material_ids[0];

		const auto& material = materials[id];
		auto diff = OApplication::Get()->GetTexturesPath(Path, UTF8ToWString(material.diffuse_texname));
		auto norm = OApplication::Get()->GetTexturesPath(Path, UTF8ToWString(material.normal_texname));
		auto height = OApplication::Get()->GetTexturesPath(Path, UTF8ToWString(material.bump_texname));
		data.Material.Name = material.name;
		if (!diff.empty())
		{
			data.Material.DiffuseMaps.push_back(diff);
		}
		if (!norm.empty())
		{
			data.Material.NormalMaps.push_back(norm);
		}
		if (!height.empty())
		{
			data.Material.HeightMaps.push_back(height);
		}
		const SMaterialSurface surf = {
			.DiffuseAlbedo = { material.ambient[0], material.ambient[1], material.ambient[2], 1.0 },
			.FresnelR0 = { material.specular[0], material.specular[1], material.specular[2] },
			.Emission = { material.emission[0], material.emission[1], material.emission[2] },
			.Roughness = 1 - (material.shininess / 100),
			.IndexOfRefraction = material.ior,
			.Dissolve = material.dissolve
		};
		data.Material.MaterialSurface = surf;
		MeshData.TotalIndices += data.Indices32.size();
		MeshData.TotalVertices += static_cast<uint32_t>(data.Vertices.size());
		MeshData.Data.push_back(std::move(data));
	}
	return true;
}