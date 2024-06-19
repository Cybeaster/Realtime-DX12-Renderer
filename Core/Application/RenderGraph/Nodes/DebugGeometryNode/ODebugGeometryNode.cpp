
#include "ODebugGeometryNode.h"

#include "Engine/Engine.h"
void ODebugGeometryNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);

	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), resource->CameraBuffer->GetGPUAddress(), pso);
}
ORenderTargetBase* ODebugGeometryNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(FindPSOInfo(PSO), GetNodeInfo().RenderLayer);
	return RenderTarget;
}