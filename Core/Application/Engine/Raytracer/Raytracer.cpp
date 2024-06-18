#include "Raytracer.h"

#include "DXR/DXRHelper.h"
#include "DXR/RaytracingPipelineGenerator.h"
#include "DXR/RootSignatureGenerator.h"
#include "DirectX/Vertex.h"
#include "Engine/Device/Device.h"
#include "PathUtils.h"

ORaytracer::ORaytracer(const SRenderTargetParams& Params)
    : ORenderTargetBase(Params)
{
}

SAccelerationStructureBuffers ORaytracer::CreateBottomLevelAS(vector<pair<ComPtr<ID3D12Resource>, uint32_t>> VertexBuffers)
{
	nv_helpers_dx12::BottomLevelASGenerator BLASGenerator;
	auto device = Device.lock()->GetDevice();
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

	BLASGenerator.ComputeASBufferSizes(device, false, &scratchSize, &resultSize);
	auto weak = weak_from_this();

	SAccelerationStructureBuffers buffers;
	buffers.Scratch = Utils::CreateResource(weak,
	                                        L"Scratch_Buffer",
	                                        device,
	                                        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                        D3D12_RESOURCE_STATE_COMMON,
	                                        SRenderConstants::DefaultHeapProperties,
	                                        scratchSize);

	buffers.Result = Utils::CreateResource(weak,
	                                       L"Result_Buffer",
	                                       device,
	                                       D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                       D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	                                       SRenderConstants::DefaultHeapProperties,
	                                       resultSize);

	auto commandList = Queue.lock()->GetCommandListAs<ID3D12GraphicsCommandList4>();
	BLASGenerator.Generate(commandList,
	                       buffers.Scratch->Resource.Get(),
	                       buffers.Result->Resource.Get(),
	                       false);
	return buffers;
}

void ORaytracer::CreateTopLevelAS(const vector<pair<TResourceInfo, DirectX::XMMATRIX>>& Instances)
{
	nv_helpers_dx12::TopLevelASGenerator topLevelASGenerator;
	auto device = Device.lock()->GetDevice();
	uint32_t counter = 0;
	for (const auto& instance : Instances)
	{
		topLevelASGenerator.AddInstance(instance.first.get()->Resource.Get(), instance.second, counter, 0);
		counter++;
	}
	UINT64 scratchSize, resultSize, instanceDescsSize;
	topLevelASGenerator.ComputeASBufferSizes(device, true, &scratchSize, &resultSize, &instanceDescsSize);
	auto weak = weak_from_this();
	TopLevelASBuffers.Scratch = Utils::CreateResource(weak,
	                                                  L"TLAS_Scratch",
	                                                  device,
	                                                  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                                  D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	                                                  SRenderConstants::DefaultHeapProperties,
	                                                  scratchSize);
	TopLevelASBuffers.Result = Utils::CreateResource(weak,
	                                                 L"TLAS_Result",
	                                                 device,
	                                                 D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	                                                 D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	                                                 SRenderConstants::DefaultHeapProperties,
	                                                 resultSize);
	TopLevelASBuffers.InstanceDesc = Utils::CreateResource(weak,
	                                                       L"TLAS_InstanceDesc",
	                                                       device,
	                                                       D3D12_RESOURCE_FLAG_NONE,
	                                                       D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                       SRenderConstants::UploadHeapProps,
	                                                       instanceDescsSize);

	topLevelASGenerator.Generate(Queue.lock()->GetCommandListAs<ID3D12GraphicsCommandList4>(),
	                             TopLevelASBuffers.Scratch->Resource.Get(),
	                             TopLevelASBuffers.Result->Resource.Get(),
	                             TopLevelASBuffers.InstanceDesc->Resource.Get());
}

void ORaytracer::CreateAccelerationStructures(const unordered_set<shared_ptr<ORenderItem>>& Items)
{
	Instances.clear();
	PROFILE_BLOCK_START(L"Create BLAS")
	for (const auto& item : Items)
	{
		if (const auto mesh = item->Geometry.lock())
		{
			auto& vertexBuffer = mesh->VertexBufferGPU;
			auto [Scratch, Result, InstanceDesc] = CreateBottomLevelAS({ { vertexBuffer, mesh->NumVertices } });
			Instances.emplace_back(Result, Load(item->GetDefaultInstance()->HlslData.World));
		}
	}
	PROFILE_BLOCK_END()

	PROFILE_BLOCK_START(L"Create TLAS")
	CreateTopLevelAS(Instances);
	PROFILE_BLOCK_END()
}

bool ORaytracer::Init(const shared_ptr<ODevice>& InDevice, const shared_ptr<OCommandQueue>& InQueue)
{
	Device = InDevice;
	Queue = InQueue;
	BuildResource();
	return true;
}

wstring ORaytracer::GetName() const
{
	return L"Raytracer";
}

TResourceInfo ORaytracer::GetTLAS() const
{
	return TopLevelASBuffers.Result;
}
uint32_t ORaytracer::GetNumSRVRequired() const
{
	return 2;
}

void ORaytracer::BuildDescriptors(IDescriptor* Descriptor)
{
	if (const auto descriptor = Cast<SRenderObjectHeap>(Descriptor))
	{
		OutputUAV = descriptor->SRVHandle.Offset();
		OutputSRV = descriptor->SRVHandle.Offset();
		BuildDescriptors();
	}
}

SDispatchPayload ORaytracer::GetDispatchPayload() const
{
	UINT numGroupsX = (UINT)ceilf(Width / 16.0f);
	UINT numGroupsY = (UINT)ceilf(Height / 16.0f);
	return { numGroupsX, numGroupsY, 1 };
}

void ORaytracer::BuildResource()
{
	OutputTexture = Utils::CreateResource(weak_from_this(), L"Raytracing_Output_Texture", Device.lock()->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, GetResourceDesc(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void ORaytracer::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	auto device = Device.lock();

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateShaderResourceView(OutputTexture, srvDesc, OutputSRV);
	device->CreateUnorderedAccessView(OutputTexture, uavDesc, OutputUAV);
}

void ORaytracer::BuildPipeline()
{
	nv_helpers_dx12::RayTracingPipelineGenerator pipelineGenerator(Device.lock()->GetDevice());
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
	return rsc.Generate(Device.lock()->GetDevice(), true);
}

ComPtr<ID3D12RootSignature> ORaytracer::CreateMissRootSignature()
{
	nv_helpers_dx12::RootSignatureGenerator rsc;
	return rsc.Generate(Device.lock()->GetDevice(), true);
}

ComPtr<ID3D12RootSignature> ORaytracer::CreateHitRootSignature()
{
	nv_helpers_dx12::RootSignatureGenerator rsc;
	return rsc.Generate(Device.lock()->GetDevice(), true);
}
SResourceInfo* ORaytracer::GetResource()
{
	return OutputTexture.get();
}