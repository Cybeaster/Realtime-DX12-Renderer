#include "TextureWaves.h"

#include "../../../Materials/Material.h"
#include "../../../Objects/GeomertryGenerator/GeometryGenerator.h"
#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"
#include "Camera/Camera.h"
#include "RenderConstants.h"
#include "RenderItem.h"
#include "Settings.h"
#include "Textures/DDSTextureLoader/DDSTextureLoader.h"

#include <DXHelper.h>
#include <Timer/Timer.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <ranges>

using namespace Microsoft::WRL;

using namespace DirectX;

OTextureWaves::OTextureWaves(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
    : OTest(_Engine, _Window)
{
}

bool OTextureWaves::Initialize()
{
	const auto engine = Engine.lock();
	assert(engine->GetCommandQueue()->GetCommandQueue());
	const auto queue = engine->GetCommandQueue();
	queue->ResetCommandList();

	Waves = engine->BuildRenderObject<OGPUWave>(engine->GetDevice().Get(),
	                                            queue->GetCommandList().Get(),
	                                            256,
	                                            256,
	                                            0.25f,
	                                            0.03f,
	                                            2.0f,
	                                            0.2f);

	engine->SetFog({ 0.7f, 0.7f, 0.7f, 1.0f }, 50.0f, 150.0f);
	CreateTexture();
	BuildQuadPatchGeometry();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildTreeSpriteGeometry();
	BuildIcosahedronGeometry();
	BuildWavesGeometryBuffers();
	BuildBoxGeometryBuffers();
	BuildMaterials();
	BuildRenderItems();

	BuildPSOTreeSprites();
	BuildPSOGeosphere();
	BuildTesselationPSO();

	engine->GetCommandQueue()->ExecuteCommandList();
	engine->FlushGPU();
	ContentLoaded = true;

	return true;
}

void OTextureWaves::UnloadContent()
{
	ContentLoaded = false;
}

void OTextureWaves::UpdateWave(const STimer& Timer) const
{
	auto commandList = Engine.lock()->GetCommandQueue()->GetCommandList();
	const auto wavesRootSignature = Engine.lock()->GetWavesRootSignature();
	static float tBase = 0.0f;
	if (Timer.GetTime() - tBase >= 0.25)
	{
		tBase += 0.25;
		int i = Utils::Math::Random(4, Waves->GetRowCount() - 5);
		int j = Utils::Math::Random(4, Waves->GetColumnCount() - 5);
		float r = Utils::Math::Random(1.f, 2.f);

		Waves->Disturb(wavesRootSignature, Engine.lock()->GetPSO(SPSOType::WavesDisturb).Get(), i, j, r);
	}
	Waves->Update(Timer, wavesRootSignature, Engine.lock()->GetPSO(SPSOType::WavesUpdate).Get());
}

void OTextureWaves::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);
	IsInputBlocked = Event.IsUIInfocus;

	auto engine = Engine.lock();
	engine->CurrentFrameResourceIndex = (engine->CurrentFrameResourceIndex + 1) % SRenderConstants::NumFrameResources;
	engine->CurrentFrameResources = engine->FrameResources[engine->CurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame
	// resource. If not, wait until the GPU has completed commands up to
	// this fence point.

	if (engine->CurrentFrameResources->Fence != 0 && engine->GetCommandQueue()->GetFence()->GetCompletedValue() < engine->CurrentFrameResources->Fence)
	{
		engine->GetCommandQueue()->WaitForFenceValue(engine->CurrentFrameResources->Fence);
	}

	AnimateMaterials(Event.Timer);
	UpdateObjectCBs(Event.Timer);
	UpdateMaterialCB();
}

void OTextureWaves::UpdateObjectCBs(const STimer& Timer)
{
	const auto engine = Engine.lock();
	const auto currentObjectCB = engine->CurrentFrameResources->ObjectCB.get();

	for (const auto& item : engine->GetAllRenderItems())
	{
		// Only update the cbuffer data if the constants have changed.
		// This needs to be tracked per frame resource.

		if (item->NumFramesDirty > 0)
		{
			auto world = XMLoadFloat4x4(&item->World);
			auto texTransform = XMLoadFloat4x4(&item->TexTransform);

			SObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.DisplacementMapTexelSize = item->DisplacementMapTexelSize;
			objConstants.GridSpatialStep = item->GridSpatialStep;
			currentObjectCB->CopyData(item->ObjectCBIndex, objConstants);

			// Next FrameResource need to ber updated too
			item->NumFramesDirty--;
		}
	}
}

void OTextureWaves::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList> CommandList, const vector<SRenderItem*>& RenderItems) const
{
	const auto engine = Engine.lock();
	const auto matCBByteSize = Utils::CalcBufferByteSize(sizeof(SMaterialConstants));
	const auto objectCBByteSize = Utils::CalcBufferByteSize(sizeof(SObjectConstants));
	const auto srv = engine->GetSRVHeap();
	const auto objectCB = engine->CurrentFrameResources->ObjectCB->GetResource();
	const auto materialCB = engine->CurrentFrameResources->MaterialCB->GetResource();
	for (size_t i = 0; i < RenderItems.size(); i++)
	{
		const auto renderItem = RenderItems[i];

		auto vertexView = renderItem->Geometry->VertexBufferView();
		auto indexView = renderItem->Geometry->IndexBufferView();

		CommandList->IASetVertexBuffers(0, 1, &vertexView);
		CommandList->IASetIndexBuffer(&indexView);
		CommandList->IASetPrimitiveTopology(renderItem->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE texDef(srv->GetGPUDescriptorHandleForHeapStart());
		texDef.Offset(renderItem->Material->DiffuseSRVHeapIndex, engine->CBVSRVUAVDescriptorSize);

		// Offset to the CBV in the descriptor heap for this object and
		// for this frame resource.

		const auto cbAddress = objectCB->GetGPUVirtualAddress() + renderItem->ObjectCBIndex * objectCBByteSize;
		const auto matCBAddress = materialCB->GetGPUVirtualAddress() + renderItem->Material->MaterialCBIndex * matCBByteSize;

		CommandList->SetGraphicsRootDescriptorTable(0, texDef);
		CommandList->SetGraphicsRootConstantBufferView(1, cbAddress);
		CommandList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		CommandList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
}

void OTextureWaves::OnMouseWheel(const MouseWheelEventArgs& Args)
{
	OTest::OnMouseWheel(Args);
	GetEngine()->GetMainPassCB().FogStart += Args.WheelDelta;
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

		//move slightly above the hill height
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

	geometry->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                       Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	                                                       vertices.data(),
	                                                       vbByteSize,
	                                                       geometry->VertexBufferUploader);

	geometry->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                      Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
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

	auto commandList = Engine.lock()->GetCommandQueue()->GetCommandList();
	auto device = Engine.lock()->GetDevice();

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

	quadSubmesh.Indices = make_unique<vector<int16_t>>(indices);
	quadSubmesh.Vertices = make_unique<vector<XMFLOAT3>>(vertices);

	geo->SetGeometry("QuadPatch", quadSubmesh);

	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OTextureWaves::CreateTexture()
{
	GetEngine()->CreateTexture("Grass", L"Resources/Textures/grass.dds");
	GetEngine()->CreateTexture("Water", L"Resources/Textures/water1.dds");
	GetEngine()->CreateTexture("Fence", L"Resources/Textures/WireFence.dds");
	GetEngine()->CreateTexture("FireBall", L"Resources/Textures/Fireball.dds");
	GetEngine()->CreateTexture("TreeArray", L"Resources/Textures/treeArray2.dds");
	GetEngine()->CreateTexture("White", L"Resources/Textures/white1x1.dds");
}

void OTextureWaves::BuildTesselationPSO()
{
	auto engine = Engine.lock();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc = engine->GetOpaquePSODesc();
	opaquePsoDesc.VS = { reinterpret_cast<BYTE*>(engine->GetShader(SShaderTypes::VSTesselation)->GetBufferPointer()),
		                 engine->GetShader(SShaderTypes::VSTesselation)->GetBufferSize() };
	opaquePsoDesc.HS = { reinterpret_cast<BYTE*>(engine->GetShader(SShaderTypes::HSTesselation)->GetBufferPointer()),
		                 engine->GetShader(SShaderTypes::HSTesselation)->GetBufferSize() };
	opaquePsoDesc.DS = { reinterpret_cast<BYTE*>(engine->GetShader(SShaderTypes::DSTesselation)->GetBufferPointer()),
		                 engine->GetShader(SShaderTypes::DSTesselation)->GetBufferSize() };
	opaquePsoDesc.PS = { reinterpret_cast<BYTE*>(engine->GetShader(SShaderTypes::PSTesselation)->GetBufferPointer()),
		                 engine->GetShader(SShaderTypes::PSTesselation)->GetBufferSize() };

	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	opaquePsoDesc.NumRenderTargets = 1;
	engine->CreatePSO(SPSOType::Tesselation, opaquePsoDesc);
}

void OTextureWaves::OnRender(const UpdateEventArgs& Event)
{
	const auto engine = Engine.lock();
	const auto commandList = engine->GetCommandQueue()->GetCommandList();

	const auto passCB = engine->CurrentFrameResources->PassCB->GetResource();
	commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	UpdateWave(Event.Timer);
	commandList->SetGraphicsRootDescriptorTable(4, Waves->GetDisplacementMap());

	GetEngine()->SetPipelineState(SPSOType::Opaque);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Opaque));

	GetEngine()->SetPipelineState(SPSOType::Tesselation);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Tesselation));

	GetEngine()->SetPipelineState(SPSOType::AlphaTested);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::AlphaTested));

	// GetEngine()->SetPipelineState(SPSOType::TreeSprites);
	// DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::AlphaTestedTreeSprites));

	GetEngine()->SetPipelineState(SPSOType::Transparent);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Transparent));

	GetEngine()->SetPipelineState(SPSOType::WavesRender);
	DrawRenderItems(commandList.Get(), engine->GetRenderItems(SRenderLayer::Waves));
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

void OTextureWaves::BuildMaterials()
{
	GetEngine()->CreateMaterial("Grass", 0, 0, { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.125f });
	GetEngine()->CreateMaterial("Water", 1, 1, { XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.0f });
	GetEngine()->CreateMaterial("WireFence", 2, 2, { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.25f });
	GetEngine()->CreateMaterial("FireBall", 3, 3, { XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.25f });
	GetEngine()->CreateMaterial("TreeSprite", 4, 4, { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.125f });
	GetEngine()->CreateMaterial("White", 5, 5, { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.5f });
}

void OTextureWaves::UpdateMaterialCB()
{
	const auto currentMaterialCB = Engine.lock()->CurrentFrameResources->MaterialCB.get();
	for (auto& materials = Engine.lock()->GetMaterials(); const auto& val : materials | std::views::values)
	{
		if (const auto material = val.get())
		{
			if (material->NumFramesDirty > 0)
			{
				const auto matTransform = XMLoadFloat4x4(&material->MaterialConsatnts.MatTransform);

				SMaterialConstants matConstants;
				matConstants.DiffuseAlbedo = material->MaterialConsatnts.DiffuseAlbedo;
				matConstants.FresnelR0 = material->MaterialConsatnts.FresnelR0;
				matConstants.Roughness = material->MaterialConsatnts.Roughness;
				XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

				currentMaterialCB->CopyData(material->MaterialCBIndex, matConstants);
				material->NumFramesDirty--;
			}
		}
	}
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
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;
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

	geo->VertexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                  Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
	                                                  vertices.data(),
	                                                  vbByteSize,
	                                                  geo->VertexBufferUploader);

	geo->IndexBufferGPU = Utils::CreateDefaultBuffer(Engine.lock()->GetDevice().Get(),
	                                                 Engine.lock()->GetCommandQueue()->GetCommandList().Get(),
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

void OTextureWaves::BuildWavesGeometryBuffers()
{
	OGeometryGenerator geoGen;
	OGeometryGenerator::SMeshData grid = geoGen.CreateGrid(160.0f, 160.0f, Waves->GetRowCount(), Waves->GetColumnCount());
	auto device = Engine.lock()->GetDevice();
	auto commandList = Engine.lock()->GetCommandQueue()->GetCommandList();
	std::vector<SVertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		vertices[i].Pos = grid.Vertices[i].Position;
		vertices[i].Normal = grid.Vertices[i].Normal;
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	std::vector<std::uint32_t> indices = grid.Indices32;

	UINT vbByteSize = Waves->GetVertexCount() * sizeof(SVertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = "WaterGeometry";

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

	geo->VertexByteStride = sizeof(SVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->SetGeometry("Grid", submesh);

	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OTextureWaves::BuildIcosahedronGeometry()
{
	OGeometryGenerator geoGen;
	OGeometryGenerator::SMeshData ico = geoGen.CreateGeosphere(10, 0);
	auto device = Engine.lock()->GetDevice();
	auto commandList = Engine.lock()->GetCommandQueue()->GetCommandList();

	std::vector<SVertex> vertices(ico.Vertices.size());
	for (size_t i = 0; i < ico.Vertices.size(); ++i)
	{
		auto& p = ico.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = ico.Vertices[i].Normal;
		vertices[i].TexC = ico.Vertices[i].TexC;
	}

	auto vbByteSize = static_cast<UINT>(vertices.size() * sizeof(SVertex));

	vector<uint16_t> indices = ico.GetIndices16();
	auto ibByteSize = static_cast<UINT>(indices.size() * sizeof(uint16_t));

	auto geometry = make_unique<SMeshGeometry>();
	geometry->Name = "Icosahedron";

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geometry->VertexBufferCPU));
	CopyMemory(geometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geometry->IndexBufferCPU));
	CopyMemory(geometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->VertexBufferGPU = Utils::CreateDefaultBuffer(device.Get(),
	                                                       commandList.Get(),
	                                                       vertices.data(),
	                                                       vbByteSize,
	                                                       geometry->VertexBufferUploader);

	geometry->IndexBufferGPU = Utils::CreateDefaultBuffer(device.Get(),
	                                                      commandList.Get(),
	                                                      indices.data(),
	                                                      ibByteSize,
	                                                      geometry->IndexBufferUploader);

	geometry->VertexByteStride = sizeof(SVertex);
	geometry->VertexBufferByteSize = vbByteSize;
	geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	geometry->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geometry->SetGeometry("Icosahedron", submesh);
	GetEngine()->SetSceneGeometry(std::move(geometry));
}

void OTextureWaves::BuildBoxGeometryBuffers()
{
	OGeometryGenerator geoGen;
	OGeometryGenerator::SMeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);
	auto device = Engine.lock()->GetDevice();
	auto commandList = Engine.lock()->GetCommandQueue()->GetCommandList();
	std::vector<SVertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SVertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<SMeshGeometry>();
	geo->Name = "BoxGeometry";

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

	geo->VertexByteStride = sizeof(SVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->SetGeometry("Box", submesh);
	GetEngine()->SetSceneGeometry(std::move(geo));
}

void OTextureWaves::BuildRenderItems()
{
	auto wavesRenderItem = make_unique<SRenderItem>();
	wavesRenderItem->World = Utils::Math::Identity4x4();
	XMStoreFloat4x4(&wavesRenderItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRenderItem->DisplacementMapTexelSize.x = 1.0f / Waves->GetColumnCount();
	wavesRenderItem->DisplacementMapTexelSize.y = 1.0f / Waves->GetRowCount();
	wavesRenderItem->GridSpatialStep = Waves->GetSpatialStep();
	wavesRenderItem->ObjectCBIndex = 0;
	wavesRenderItem->Geometry = GetEngine()->GetSceneGeometry()["WaterGeometry"].get();
	wavesRenderItem->Material = GetEngine()->FindMaterial("Water");
	wavesRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRenderItem->IndexCount = wavesRenderItem->Geometry->FindSubmeshGeomentry("Grid")->IndexCount;
	wavesRenderItem->StartIndexLocation = wavesRenderItem->Geometry->FindSubmeshGeomentry("Grid")->StartIndexLocation;
	wavesRenderItem->BaseVertexLocation = wavesRenderItem->Geometry->FindSubmeshGeomentry("Grid")->BaseVertexLocation;

	WavesRenderItem = wavesRenderItem.get();
	GetEngine()->AddRenderItem(SRenderLayer::Waves, std::move(wavesRenderItem));

	auto gridRenderItem = make_unique<SRenderItem>();
	gridRenderItem->World = Utils::Math::Identity4x4();
	XMStoreFloat4x4(&gridRenderItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRenderItem->ObjectCBIndex = 1;
	gridRenderItem->Geometry = GetEngine()->GetSceneGeometry()["LandGeo"].get();
	gridRenderItem->Material = GetEngine()->FindMaterial("Grass");
	gridRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->IndexCount = gridRenderItem->Geometry->FindSubmeshGeomentry("Grid")->IndexCount;
	gridRenderItem->StartIndexLocation = gridRenderItem->Geometry->FindSubmeshGeomentry("Grid")->StartIndexLocation;
	gridRenderItem->BaseVertexLocation = gridRenderItem->Geometry->FindSubmeshGeomentry("Grid")->BaseVertexLocation;
	XMStoreFloat4x4(&gridRenderItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));

	GetEngine()->AddRenderItem(SRenderLayer::Opaque, std::move(gridRenderItem));

	auto boxRenderItem = make_unique<SRenderItem>();
	XMStoreFloat4x4(&boxRenderItem->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	boxRenderItem->ObjectCBIndex = 2;
	boxRenderItem->Geometry = GetEngine()->GetSceneGeometry()["BoxGeometry"].get();
	boxRenderItem->Material = GetEngine()->FindMaterial("WireFence");
	boxRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRenderItem->IndexCount = boxRenderItem->Geometry->FindSubmeshGeomentry("Box")->IndexCount;
	boxRenderItem->StartIndexLocation = boxRenderItem->Geometry->FindSubmeshGeomentry("Box")->StartIndexLocation;
	boxRenderItem->BaseVertexLocation = boxRenderItem->Geometry->FindSubmeshGeomentry("Box")->BaseVertexLocation;

	GetEngine()->AddRenderItem(SRenderLayer::AlphaTested, std::move(boxRenderItem));

	auto treeRenderItem = make_unique<SRenderItem>();
	treeRenderItem->World = Utils::Math::Identity4x4();
	treeRenderItem->ObjectCBIndex = 3;
	treeRenderItem->Geometry = GetEngine()->FindSceneGeometry("TreeSprites");
	treeRenderItem->Material = GetEngine()->FindMaterial("TreeSprite");
	treeRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	treeRenderItem->IndexCount = treeRenderItem->Geometry->FindSubmeshGeomentry("Points")->IndexCount;
	treeRenderItem->StartIndexLocation = treeRenderItem->Geometry->FindSubmeshGeomentry("Points")->StartIndexLocation;
	treeRenderItem->BaseVertexLocation = treeRenderItem->Geometry->FindSubmeshGeomentry("Points")->BaseVertexLocation;
	GetEngine()->AddRenderItem(SRenderLayer::AlphaTestedTreeSprites, std::move(treeRenderItem));

	auto icosahedronRenderItem = make_unique<SRenderItem>();
	XMStoreFloat4x4(&icosahedronRenderItem->World, XMMatrixTranslation(0.0f, 10.f, 10));
	icosahedronRenderItem->ObjectCBIndex = 4;
	icosahedronRenderItem->Geometry = GetEngine()->FindSceneGeometry("Icosahedron");
	icosahedronRenderItem->Material = GetEngine()->FindMaterial("FireBall");
	icosahedronRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	auto geometry = icosahedronRenderItem->Geometry->FindSubmeshGeomentry("Icosahedron");
	icosahedronRenderItem->IndexCount = geometry->IndexCount;
	icosahedronRenderItem->StartIndexLocation = geometry->StartIndexLocation;
	icosahedronRenderItem->BaseVertexLocation = geometry->BaseVertexLocation;
	GetEngine()->AddRenderItem(SRenderLayer::IcosahedronLODs, std::move(icosahedronRenderItem));

	/*auto quadPatchRitem = std::make_unique<SRenderItem>();

	XMStoreFloat4x4(&quadPatchRitem->World, XMMatrixScaling(2.0f, 1.0f, 2.0f) * XMMatrixTranslation(0.0f, 15.0f, 0.f));

	quadPatchRitem->TexTransform = Utils::Math::Identity4x4();
	quadPatchRitem->ObjectCBIndex = 5;
	quadPatchRitem->Material = GetEngine()->FindMaterial("White");
	quadPatchRitem->Geometry = GetEngine()->FindSceneGeometry("QuadPatch");
	quadPatchRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
	auto submesh = quadPatchRitem->Geometry->FindSubmeshGeomentry("QuadPatch");
	quadPatchRitem->IndexCount = submesh.IndexCount;
	quadPatchRitem->StartIndexLocation = submesh.StartIndexLocation;
	quadPatchRitem->BaseVertexLocation = submesh.BaseVertexLocation;
	GetEngine()->AddRenderItem(SRenderLayer::Tesselation, std::move(quadPatchRitem));*/

	auto bquadPatchRitem = std::make_unique<SRenderItem>();
	XMStoreFloat4x4(&bquadPatchRitem->World, XMMatrixScaling(2.0f, 1.0f, 2.0f) * XMMatrixTranslation(0.0f, 15.0f, 0.f));
	bquadPatchRitem->TexTransform = Utils::Math::Identity4x4();
	bquadPatchRitem->ObjectCBIndex = 5;
	bquadPatchRitem->Material = Engine.lock()->FindMaterial("White");
	bquadPatchRitem->Geometry = Engine.lock()->FindSceneGeometry("QuadPatch");
	bquadPatchRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
	auto submesh = bquadPatchRitem->Geometry->FindSubmeshGeomentry("QuadPatch");
	bquadPatchRitem->IndexCount = submesh->IndexCount;
	bquadPatchRitem->StartIndexLocation = submesh->StartIndexLocation;
	bquadPatchRitem->BaseVertexLocation = submesh->BaseVertexLocation;
	GetEngine()->AddRenderItem(SRenderLayer::Tesselation, std::move(bquadPatchRitem));
}

float OTextureWaves::GetHillsHeight(float X, float Z) const
{
	return 0.3 * (Z * sinf(0.1f * X) + X * cosf(0.1f * Z));
}

void OTextureWaves::AnimateMaterials(const STimer& Timer)
{
	auto waterMaterial = GetEngine()->FindMaterial("Water");

	float& tu = waterMaterial->MaterialConsatnts.MatTransform(3, 0);
	float& tv = waterMaterial->MaterialConsatnts.MatTransform(3, 1);

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

	waterMaterial->MaterialConsatnts.MatTransform(3, 0) = tu;
	waterMaterial->MaterialConsatnts.MatTransform(3, 1) = tv;

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

#pragma optimize("", on)
