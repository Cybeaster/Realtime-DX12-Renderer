#pragma once
#include "DXR/BottomLevelASGenerator.h"
#include "DXR/TopLevelASGenerator.h"
#include "DirectX/DXHelper.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"

#include <dxcapi.h>

class OCommandQueue;
class ODevice;
struct SAccelerationStructureBuffers
{
	SResourceInfo Scratch; // Scratch memory for AS builder
	SResourceInfo Result; // Where the AS is
	SResourceInfo InstanceDesc; // Hold the matrices of the instances
};

class ORaytracer : public IRenderObject
{
public:
	SAccelerationStructureBuffers CreateBottomLevelAS(vector<pair<ComPtr<ID3D12Resource>, uint32_t>> VertexBuffers);
	void CreateTopLevelAS(const vector<pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& Instances);
	void CreateAccelerationStructures();
	void CreateAccelerationStructure();
	bool Init(ODevice* InDevice, OCommandQueue* InQueue);
	wstring GetName() override;

private:
	void BuildPipeline();
	ComPtr<ID3D12RootSignature> CreateRaygenRootSignature();
	ComPtr<ID3D12RootSignature> CreateMissRootSignature();
	ComPtr<ID3D12RootSignature> CreateHitRootSignature();

	ComPtr<ID3D12RootSignature> RaygenRootSignature;
	ComPtr<ID3D12RootSignature> MissRootSignature;
	ComPtr<ID3D12RootSignature> HitRootSignature;

	ComPtr<IDxcBlob> RaygenShader;
	ComPtr<IDxcBlob> MissShader;
	ComPtr<IDxcBlob> HitShader;

	ComPtr<ID3D12StateObject> Pipeline;

	ComPtr<ID3D12StateObjectProperties> PipelineInfo;

	ComPtr<ID3D12Resource> BottomLevelAS;
	nv_helpers_dx12::TopLevelASGenerator TopLevelASGenerator;
	SAccelerationStructureBuffers TopLevelASBuffers;
	vector<pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> Instances;
	ODevice* Device = nullptr;
	OCommandQueue* Queue = nullptr;
};
