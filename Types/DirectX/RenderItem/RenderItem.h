#pragma once
#include "../../Materials/Material.h"
#include "..\..\Utils\Math.h"
#include "DXHelper.h"
#include "InstanceData.h"
#include "Logger.h"
#include "RenderConstants.h"

struct SFrameResource;
struct ORenderItem
{
	ORenderItem() = default;
	ORenderItem(ORenderItem&&) = default;
	ORenderItem(const ORenderItem&) = default;

	void BindResources(ID3D12GraphicsCommandList* CmdList, SFrameResource* Frame) const;

	void UpdateWorldMatrix(const DirectX::XMMATRIX& WorldMatrix);
	bool IsValid() const;
	bool IsValidChecked() const;

	string RenderLayer = "NONE";
	bool bTraceable = true;
	bool bFrustrumCoolingEnabled = true;

	DirectX::XMFLOAT4X4 World = Utils::Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Utils::Math::Identity4x4();

	uint32_t NumFramesDirty = SRenderConstants::NumFrameResources;

	UINT ObjectCBIndex = -1;

	SMaterial* Material = nullptr;
	SMeshGeometry* Geometry = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	DirectX::BoundingBox Bounds;
	vector<SInstanceData> Instances;

	UINT IndexCount = 0;
	UINT VisibleInstanceCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	int32_t StartInstanceLocation = 0;
};

struct SRenderItemParams
{
	SRenderItemParams(SMaterial* Material)
	    : MaterialParams(Material) {}
	SRenderItemParams() = default;

	SRenderItemParams(SMaterial* Material, size_t Instances)
	    : MaterialParams(Material), NumberOfInstances(Instances) {}

	string Submesh;
	SMaterialParams MaterialParams;
	size_t NumberOfInstances = 1;
	bool bFrustrumCoolingEnabled = false;
	bool Pickable = false;
};