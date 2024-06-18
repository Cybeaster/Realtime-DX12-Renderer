
#include "RaytracingNode.h"

#include "Engine/Engine.h"
ORenderTargetBase* ORaytracingNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto payload = Raytracer->GetDispatchPayload();
	CommandQueue->Dispatch(payload.Width, payload.Height, payload.Depth);
	return RenderTarget;
}

void ORaytracingNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), resource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(TLAS), Raytracer->GetTLAS()->Resource->GetGPUVirtualAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(OUTPUT), Raytracer->GetUAV().GPUHandle, pso);
}