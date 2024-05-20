#include "Raytracer.h"

#include "DXR/DXRHelper.h"
#include "DXR/RaytracingPipelineGenerator.h"
#include "DXR/RootSignatureGenerator.h"
#include "DirectX/Vertex.h"
#include "Engine/Device/Device.h"
#include "PathUtils.h"

SAccelerationStructureBuffers ORaytracer::CreateBottomLevelAS(vector<pair<ComPtr<ID3D12Resource>, uint32_t>> VertexBuffers)
{
	nv_helpers_dx12::BottomLevelASGenerator BLASGenerator;

	for (const auto& buffer : VertexBuffers)
	{
		BLASGenerator.AddVertexBuffer(buffer.first.Get(),
		                              0,
		                              buffer.second,
		                              sizeof(SVertex),
		                              nullptr,
		                              0);
	}
	uint64_t scratchSize = 0;
	uint64_t resultSize = 0;

	BLASGenerator.ComputeASBufferSizes(Device->GetDevice(), false, &scratchSize, &resultSize);
	auto weak = weak_from_this();

	SAccelerationStructureBuffers buffers;
	buffers.Scratch = Utils::CreateResource(weak,
	                                        L"Scratch_Buffer",
	                                        Device->GetDevice(),
	                                        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                        D3D12_RESOURCE_STATE_COMMON,
	                                        SRenderConstants::DefaultHeapProperties,
	                                        scratchSize);

	buffers.Result = Utils::CreateResource(weak,
	                                       L"Result_Buffer",
	                                       Device->GetDevice(),
	                                       D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                       D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	                                       SRenderConstants::DefaultHeapProperties,
	                                       resultSize);

	BLASGenerator.Generate(Queue->GetCommandListAs<ID3D12GraphicsCommandList4>(),
	                       buffers.Scratch->Resource.Get(),
	                       buffers.Result->Resource.Get(),
	                       false);
	return buffers;
}

void ORaytracer::CreateTopLevelAS(const vector<pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& Instances)
{
	uint32_t counter = 0;
	for (const auto& instance : Instances)
	{
		TopLevelASGenerator.AddInstance(instance.first.Get(), instance.second, counter, 0);
		counter++;
	}
	UINT64 scratchSize, resultSize, instanceDescsSize;
	TopLevelASGenerator.ComputeASBufferSizes(Device->GetDevice(), true, &scratchSize, &resultSize, &instanceDescsSize);
	auto weak = weak_from_this();
	TopLevelASBuffers.Scratch = Utils::CreateResource(weak,
	                                                  L"TLAS_Scratch",
	                                                  Device->GetDevice(),
	                                                  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                                  D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	                                                  SRenderConstants::DefaultHeapProperties,
	                                                  scratchSize);
	TopLevelASBuffers.Result = Utils::CreateResource(weak,
	                                                 L"TLAS_Result",
	                                                 Device->GetDevice(),
	                                                 D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                                 D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	                                                 SRenderConstants::DefaultHeapProperties,
	                                                 resultSize);
	TopLevelASBuffers.InstanceDesc = Utils::CreateResource(weak,
	                                                       L"TLAS_InstanceDesc",
	                                                       Device->GetDevice(),
	                                                       D3D12_RESOURCE_FLAG_NONE,
	                                                       D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                       SRenderConstants::UploadHeapProps,
	                                                       instanceDescsSize);

	TopLevelASGenerator.Generate(Queue->GetCommandListAs<ID3D12GraphicsCommandList4>(),
	                             TopLevelASBuffers.Scratch->Resource.Get(),
	                             TopLevelASBuffers.Result->Resource.Get(),
	                             TopLevelASBuffers.InstanceDesc->Resource.Get());
}

void ORaytracer::CreateAccelerationStructure()
{
	SAccelerationStructureBuffers BLASBuffers = CreateBottomLevelAS({});
}

bool ORaytracer::Init(ODevice* InDevice, OCommandQueue* InQueue)
{
	Device = InDevice;
	Queue = InQueue;
	BuildPipeline();
	return true;
}
wstring ORaytracer::GetName() const
{
	return L"Raytracer";
}

void ORaytracer::BuildPipeline()
{
	nv_helpers_dx12::RayTracingPipelineGenerator pipelineGenerator(Device->GetDevice());
	auto rootShaderFolder = OApplication::Get()->GetRootShaderFolder();
	RaygenShader = nv_helpers_dx12::CompileShaderLibrary((rootShaderFolder + L"Raygen.hlsl").c_str());
	MissShader = nv_helpers_dx12::CompileShaderLibrary((rootShaderFolder + L"Miss.hlsl").c_str());
	HitShader = nv_helpers_dx12::CompileShaderLibrary((rootShaderFolder + L"Hit.hlsl").c_str());

	pipelineGenerator.AddLibrary(RaygenShader.Get(), { L"Raygen" });
	pipelineGenerator.AddLibrary(MissShader.Get(), { L"Miss" });
	pipelineGenerator.AddLibrary(HitShader.Get(), { L"ClosestHit" });

	RaygenRootSignature = CreateRaygenRootSignature();
	MissRootSignature = CreateMissRootSignature();
	HitRootSignature = CreateHitRootSignature();
	pipelineGenerator.AddHitGroup(L"HitGroup", L"ClosestHit");

	pipelineGenerator.AddRootSignatureAssociation(RaygenRootSignature.Get(), { L"Raygen" });
	pipelineGenerator.AddRootSignatureAssociation(MissRootSignature.Get(), { L"Miss" });
	pipelineGenerator.AddRootSignatureAssociation(HitRootSignature.Get(), { L"HitGroup" });

	pipelineGenerator.SetMaxPayloadSize(4 * sizeof(float)); // RGB + distance
	pipelineGenerator.SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates
	pipelineGenerator.SetMaxRecursionDepth(1);
	Pipeline = pipelineGenerator.Generate();
	THROW_IF_FAILED(Pipeline->QueryInterface(IID_PPV_ARGS(&PipelineInfo)));
}

ComPtr<ID3D12RootSignature> ORaytracer::CreateRaygenRootSignature()
{
	nv_helpers_dx12::RootSignatureGenerator rsc;
	rsc.AddHeapRangesParameter(
	    { { 0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/, D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/, 0 /*heap slot where the UAV is defined*/ },
	      { 0 /*t0*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/, 1 } });
	return rsc.Generate(Device->GetDevice(), true);
}

ComPtr<ID3D12RootSignature> ORaytracer::CreateMissRootSignature()
{
	nv_helpers_dx12::RootSignatureGenerator rsc;
	return rsc.Generate(Device->GetDevice(), true);
}

ComPtr<ID3D12RootSignature> ORaytracer::CreateHitRootSignature()
{
	nv_helpers_dx12::RootSignatureGenerator rsc;
	return rsc.Generate(Device->GetDevice(), true);
}