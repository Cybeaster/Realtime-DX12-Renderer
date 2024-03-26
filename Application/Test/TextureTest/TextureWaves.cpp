#include "TextureWaves.h"

#include "EngineHelper.h"
#include "Geometry/GPUWave/GpuWave.h"

#include <Timer/Timer.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <ranges>
using namespace Microsoft::WRL;

using namespace DirectX;

OTextureWaves::OTextureWaves(OWindow* _Window)
    : OTest(_Window)
{
}

bool OTextureWaves::Initialize()
{
	const auto engine = Engine;
	const auto queue = engine->GetCommandQueue();
	Waves = engine->BuildRenderObject<OGPUWave>(ERenderGroup::RenderTargets, engine->GetDevice().Get(), queue->GetCommandList().Get(), 256, 256, 0.25f, 0.03f, 2.0f, 0.2f);
	BuildRenderItems();
	return true;
}

void OTextureWaves::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);
	IsInputBlocked = Event.IsUIInfocus;

	UpdateMaterialCB();
}

void OTextureWaves::BuildTreeSpriteGeometry()
{
	struct STreeSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int treeCount = 16;
	array<STreeSpriteVertex, 16> vertices;

	for (UINT i = 0; i < treeCount; ++i)
	{
		float x = Utils::Math::Random(-45.0f, 45.0f);
		float z = Utils::Math::Random(-45.0f, 45.0f);
		float y = GetHillsHeight(x, z);

		// move slightly above the hill height
		y += 10.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);
	}

	array<uint16_t, 16> indices = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};

	const UINT vbByteSize = vertices.size() * sizeof(STreeSpriteVertex);
	const UINT ibByteSize = indices.size() * sizeof(uint16_t);

	auto geometry = make_unique<SMeshGeometry>();
	geometry->Name = "TreeSprites";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geometry->VertexBufferCPU));
	CopyMemory(geometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geometry->IndexBufferCPU));
	CopyMemory(geometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine->GetDevice().Get(),
	                                                       Engine->GetCommandQueue()->GetCommandList().Get(),
	                                                       vertices.data(),
	                                                       vbByteSize,
	                                                       geometry->VertexBufferUploader);

	geometry->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine->GetDevice().Get(),
	                                                      Engine->GetCommandQueue()->GetCommandList().Get(),
	                                                      indices.data(),
	                                                      ibByteSize,
	                                                      geometry->IndexBufferUploader);

	geometry->VertexByteStride = sizeof(STreeSpriteVertex);
	geometry->VertexBufferByteSize = vbByteSize;
	geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	geometry->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geometry->SetGeometry("Points", submesh);
	GetEngine()->SetSceneGeometry(std::move(geometry));
}

void OTextureWaves::BuildQuadPatchGeometry()
{
	std::vector<XMFLOAT3> vertices = {
		// Row 0
		XMFLOAT3(-5.0f, -5.0f, +10.0f),
		XMFLOAT3(-0.0f, 0.0f, +15.0f),
		XMFLOAT3(+5.0f, 0.0f, +15.0f),
		XMFLOAT3(+10.0f, 0.0f, +15.0f),

		// Row 1
		XMFLOAT3(-15.0f, 0.0f, +5.0f),
		XMFLOAT3(-5.0f, 0.0f, +5.0f),
		XMFLOAT3(+5.0f, 20.0f, +5.0f),
		XMFLOAT3(+15.0f, 0.0f, +5.0f),

		// Row 2
		XMFLOAT3(-15.0f, 0.0f, -5.0f),
		XMFLOAT3(-5.0f, 0.0f, -5.0f),
		XMFLOAT3(+5.0f, 0.0f, -5.0f),
		XMFLOAT3(+15.0f, 0.0f, -5.0f),

		// Row 3
		XMFLOAT3(-10.0f, 10.0f, -15.0f),
		XMFLOAT3(-5.0f, 0.0f, -15.0f),
		XMFLOAT3(+5.0f, 0.0f, -15.0f),
		XMFLOAT3(+25.0f, 10.0f, -15.0f)
	};

	vector<std::int16_t> indices = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto commandList = Engine->GetCommandQueue()->GetCommandList();
	auto device = Engine->GetDevice();

	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = "QuadPatch";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(device.Get(),
	                                                  commandList.Get(),
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(device.Get(),
	                                                 commandList.Get(),
	                                                 indices.data(),
	                                                 ibByteSize,
	                                                 geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(XMFLOAT3);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = (UINT)indices.size();
	quadSubmesh.StartIndexLocation = 0;
	quadSubmesh.BaseVertexLocation = 0;

	/*quadSubmesh.Indices = make_unique<vector<int16_t>>(indices);
	quadSubmesh.Vertices = make_unique<vector<XMFLOAT3>>(vertices);*/

	geo->SetGeometry("QuadPatch", quadSubmesh);

	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OTextureWaves::BuildPSOTreeSprites()
{
	UINT state;
	bool enable = GetEngine()->GetMSAAState(state);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePSO;
	ZeroMemory(&treeSpritePSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	auto defaultInputLayout = GetEngine()->GetDefaultInputLayout();
	auto defaultRootSignature = GetEngine()->GetDefaultRootSignature();

	treeSpritePSO.InputLayout = { TreeSpriteInputLayout.data(), static_cast<UINT>(TreeSpriteInputLayout.size()) };
	treeSpritePSO.pRootSignature = defaultRootSignature;

	auto vsShader
	    = GetEngine()->GetShader(SShaderTypes::VSTreeSprite);
	auto psShader = GetEngine()->GetShader(SShaderTypes::PSTreeSprite);
	auto gsShader = GetEngine()->GetShader(SShaderTypes::GSTreeSprite);

	treeSpritePSO.VS = { reinterpret_cast<BYTE*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	treeSpritePSO.PS = { reinterpret_cast<BYTE*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	treeSpritePSO.GS = { reinterpret_cast<BYTE*>(gsShader->GetBufferPointer()), gsShader->GetBufferSize() };

	treeSpritePSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	treeSpritePSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	treeSpritePSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	treeSpritePSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	treeSpritePSO.SampleMask = UINT_MAX;
	treeSpritePSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePSO.NumRenderTargets = 1;
	treeSpritePSO.RTVFormats[0] = SRenderConstants::BackBufferFormat;
	treeSpritePSO.SampleDesc.Count = enable ? state - 1 : 1;
	treeSpritePSO.SampleDesc.Quality = enable ? (state - 1) : 0;
	treeSpritePSO.DSVFormat = SRenderConstants::DepthBufferFormat;
	GetEngine()->CreatePSO(SPSOType::TreeSprites, treeSpritePSO);
}

void OTextureWaves::BuildPSOGeosphere()
{
	UINT state;
	bool enable = GetEngine()->GetMSAAState(state);

	auto defaultInputLayout = GetEngine()->GetDefaultInputLayout();
	auto defaultRootSignature = GetEngine()->GetDefaultRootSignature();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC IcosahedronPSO;
	ZeroMemory(&IcosahedronPSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	IcosahedronPSO.InputLayout = { defaultInputLayout.data(), static_cast<UINT>(defaultInputLayout.size()) };
	IcosahedronPSO.pRootSignature = defaultRootSignature;

	auto vsShader = GetEngine()->GetShader(SShaderTypes::VSIcosahedron);
	auto psShader = GetEngine()->GetShader(SShaderTypes::PSIcosahedron);
	auto gsShader = GetEngine()->GetShader(SShaderTypes::GSIcosahedron);

	IcosahedronPSO.VS = { reinterpret_cast<BYTE*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	IcosahedronPSO.PS = { reinterpret_cast<BYTE*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	IcosahedronPSO.GS = { reinterpret_cast<BYTE*>(gsShader->GetBufferPointer()), gsShader->GetBufferSize() };

	IcosahedronPSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	IcosahedronPSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	IcosahedronPSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	IcosahedronPSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	IcosahedronPSO.SampleMask = UINT_MAX;
	IcosahedronPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	IcosahedronPSO.NumRenderTargets = 1;
	IcosahedronPSO.RTVFormats[0] = SRenderConstants::BackBufferFormat;
	IcosahedronPSO.SampleDesc.Count = enable ? state - 1 : 1;
	IcosahedronPSO.SampleDesc.Quality = enable ? (state - 1) : 0;
	IcosahedronPSO.DSVFormat = SRenderConstants::DepthBufferFormat;
	GetEngine()->CreatePSO(SPSOType::Icosahedron, IcosahedronPSO);
}

void OTextureWaves::UpdateMaterialCB()
{
}

void OTextureWaves::BuildShadersAndInputLayout()
{
	TreeSpriteInputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void OTextureWaves::BuildLandGeometry()
{
	OGeometryGenerator generator;
	auto grid = generator.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// Extract the vertex elements we are interested and apply the height
	// function to each vertex. In addition, color the vertices based on
	// their height so we have sandy looking beaches, grassy low hills,
	// and snow mountain peaks.
	//
	std::vector<SVertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Position = p;
		vertices[i].Position.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;
		vertices[i].TangentU = grid.Vertices[i].TangentU;
	}

	const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(SVertex));
	const std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(std::uint16_t));

	auto geo = make_unique<SMeshGeometry>();
	geo->Name = "LandGeo";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine->GetDevice().Get(),
	                                                  Engine->GetCommandQueue()->GetCommandList().Get(),
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine->GetDevice().Get(),
	                                                 Engine->GetCommandQueue()->GetCommandList().Get(),
	                                                 indices.data(),
	                                                 ibByteSize,
	                                                 geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry geometry;
	geometry.IndexCount = static_cast<UINT>(indices.size());
	geometry.StartIndexLocation = 0;
	geometry.BaseVertexLocation = 0;
	geo->SetGeometry("Grid", geometry);

	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OTextureWaves::BuildRenderItems()
{
	const auto engine = Engine;
	SRenderItemParams carParams;
	carParams.NumberOfInstances = 1;
	carParams.MaterialParams = { FindMaterial(SMaterialNames::Bronze) };

	auto mesh = CreateMesh("Car",
	                       "Resources/Models/car.txt",
	                       EParserType::Custom,
	                       ETextureMapType::None);

	auto car = engine->BuildRenderItemFromMesh(SRenderLayer::Opaque, mesh, carParams);

	SRenderItemParams highlightedParams;
	highlightedParams.NumberOfInstances = 1;
	highlightedParams.MaterialParams = { FindMaterial(SMaterialNames::Picked) };
	highlightedParams.Pickable = false;

	engine->BuildPickRenderItem();
	CreateGridRenderItem(SRenderLayer::Waves,
	                     "WaterNode",
	                     160,
	                     160,
	                     Waves->GetRowCount(),
	                     Waves->GetColumnCount(),
	                     Waves->GetRIParams());

	constexpr size_t n = 2;
	SRenderItemParams params;
	params.Pickable = true;
	params.NumberOfInstances = n * n * n;
	params.MaterialParams = { FindMaterial(SMaterialNames::Debug) };
	auto skull = engine->BuildRenderItemFromMesh(SRenderLayer::Opaque,
	                                             "Skull",
	                                             "Resources/Models/skull.txt",
	                                             EParserType::Custom,
	                                             ETextureMapType::Spherical,
	                                             params);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				const int idx = k * n * n + i * n + j;
				skull->Instances[idx].World = XMFLOAT4X4(
				    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&skull->Instances[idx].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				const auto nextMatIdx = idx % GetMaterials().size();
				skull->Instances[idx].MaterialIndex = nextMatIdx;
			}
		}
	}
}

float OTextureWaves::GetHillsHeight(float X, float Z) const
{
	return 0.3 * (Z * sinf(0.1f * X) + X * cosf(0.1f * Z));
}

void OTextureWaves::AnimateMaterials(const STimer& Timer)
{
	const auto waterMaterial = FindMaterial(STextureNames::Water);

	float& tu = waterMaterial->MatTransform(3, 0);
	float& tv = waterMaterial->MatTransform(3, 1);

	tu += 0.1f * Timer.GetDeltaTime();
	tv += 0.02f * Timer.GetDeltaTime();

	if (tu >= 1.0)
	{
		tu -= 1.0f;
	}

	if (tv >= 1.0)
	{
		tv -= 1.0f;
	}

	waterMaterial->MatTransform(3, 0) = tu;
	waterMaterial->MatTransform(3, 1) = tv;

	waterMaterial->NumFramesDirty = SRenderConstants::NumFrameResources;
}

XMFLOAT3 OTextureWaves::GetHillsNormal(float X, float Z) const
{
	XMFLOAT3 n(
	    -0.03f * Z * cosf(0.1f * X) - 0.3f * cosf(0.1f * Z),
	    1.0f,
	    -0.3f * sinf(0.1f * X) + 0.03f * X * sinf(0.1f * Z));

	const XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);
	return n;
}
