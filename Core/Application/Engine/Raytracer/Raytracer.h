#pragma once
#include "DXR/BottomLevelASGenerator.h"
#include "DXR/TopLevelASGenerator.h"
#include "DirectX/DXHelper.h"
#include "DirectX/Vertex.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "Engine/RenderTarget/RenderTarget.h"

#include <dxcapi.h>

class OCommandQueue;
class ODevice;
struct SAccelerationStructureBuffers
{
	TResourceInfo Scratch; // Scratch memory for AS builder
	TResourceInfo Result; // Where the AS is
	TResourceInfo InstanceDesc; // Hold the matrices of the instances
};

struct SDispatchPayload
{
	LONG Width;
	LONG Height;
	LONG Depth;
};

struct SAccelerationMeshInfo
{
	uint32_t MeshIndex = 0;
	uint32_t VertexCount = 0;
	uint32_t VertexOffset = 0;
};

class ORaytracer : public ORenderTargetBase
{
public:
	explicit ORaytracer(const SRenderTargetParams& Params);
	SAccelerationStructureBuffers CreateBottomLevelAS(const vector<pair<ComPtr<ID3D12Resource>, uint32_t>>& VertexBuffers, size_t Offset);
	void CreateTopLevelAS(const vector<pair<SAccelerationStructureBuffers, DirectX::XMMATRIX>>& Instances);
	void CreateAccelerationStructures(const unordered_set<shared_ptr<ORenderItem>>& Items);
	bool Init(const shared_ptr<ODevice>& InDevice, const shared_ptr<OCommandQueue>& InQueue);
	wstring GetName() const override;
	TResourceInfo GetTLAS() const;
	uint32_t GetNumSRVRequired() const override;
	void BuildDescriptors(IDescriptor* Descriptor) override;
	auto GetUAV() const { return OutputUAV; }
	auto GetSRV() const { return OutputSRV; }
	SDispatchPayload GetDispatchPayload() const;
	SDescriptorPair GetTLASSRV() const;

private:
	void CreateBuffers(const unordered_set<shared_ptr<ORenderItem>>& Items);
	void BuildResource() override;
	void BuildDescriptors() override;

public:
	SResourceInfo* GetResource() override;
	TUploadBuffer<HLSL::VertexData> VertexBuffer;
	TUploadBuffer<HLSL::InstanceData> InstanceBuffer; //Move to frame resource
private:
	ComPtr<ID3D12RootSignature> RaygenRootSignature;
	ComPtr<ID3D12RootSignature> MissRootSignature;
	ComPtr<ID3D12RootSignature> HitRootSignature;
	ComPtr<ID3D12StateObject> Pipeline;

	ComPtr<ID3D12StateObjectProperties> PipelineInfo;

	SAccelerationStructureBuffers TopLevelASBuffers;
	vector<pair<SAccelerationStructureBuffers, DirectX::XMMATRIX>> BottomLevelASs;
	weak_ptr<ODevice> Device;
	weak_ptr<OCommandQueue> Queue;

	TResourceInfo OutputTexture;
	SDescriptorPair TLASSRV;
	SDescriptorPair OutputUAV;
	SDescriptorPair OutputSRV;
};
