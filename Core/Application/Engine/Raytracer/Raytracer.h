#pragma once
#include "DXR/BottomLevelASGenerator.h"
#include "DXR/TopLevelASGenerator.h"
#include "DirectX/DXHelper.h"
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

class ORaytracer : public ORenderTargetBase
{
public:
	explicit ORaytracer(const SRenderTargetParams& Params);
	SAccelerationStructureBuffers CreateBottomLevelAS(vector<pair<ComPtr<ID3D12Resource>, uint32_t>> VertexBuffers);
	void CreateTopLevelAS(const vector<pair<TResourceInfo, DirectX::XMMATRIX>>& Instances);
	void CreateAccelerationStructures(const unordered_set<shared_ptr<ORenderItem>>& Items);
	bool Init(const shared_ptr<ODevice>& InDevice, const shared_ptr<OCommandQueue>& InQueue);
	wstring GetName() const override;
	TResourceInfo GetTLAS() const;
	uint32_t GetNumSRVRequired() const override;
	void BuildDescriptors(IDescriptor* Descriptor) override;
	auto GetUAV() const { return OutputUAV; }
	auto GetSRV() const { return OutputSRV; }
	SDispatchPayload GetDispatchPayload() const;
private:
	void BuildResource() override;
	void BuildDescriptors() override;
	void BuildPipeline();

	ComPtr<ID3D12RootSignature> CreateRaygenRootSignature();
	ComPtr<ID3D12RootSignature> CreateMissRootSignature();
	ComPtr<ID3D12RootSignature> CreateHitRootSignature();

public:
	SResourceInfo* GetResource() override;

private:
	ComPtr<ID3D12RootSignature> RaygenRootSignature;
	ComPtr<ID3D12RootSignature> MissRootSignature;
	ComPtr<ID3D12RootSignature> HitRootSignature;

	ComPtr<IDxcBlob> RaygenShader;
	ComPtr<IDxcBlob> MissShader;
	ComPtr<IDxcBlob> HitShader;

	ComPtr<ID3D12StateObject> Pipeline;

	ComPtr<ID3D12StateObjectProperties> PipelineInfo;

	ComPtr<ID3D12Resource> BottomLevelAS;
	SAccelerationStructureBuffers TopLevelASBuffers;
	vector<pair<TResourceInfo, DirectX::XMMATRIX>> Instances;
	weak_ptr<ODevice> Device;
	weak_ptr<OCommandQueue> Queue;

	TResourceInfo OutputTexture;
	SDescriptorPair OutputUAV;
	SDescriptorPair OutputSRV;
};
