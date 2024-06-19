#include "AabbVisNode.h"

#include "CommandQueue/CommandQueue.h"
#include "DirectX/HLSL/HlslTypes.h"
#include "Engine/Engine.h"

void OAABBVisNode::SetupCommonResources()
{
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), OEngine::Get()->CurrentFrameResource->CameraBuffer->GetGPUAddress(), pso);
}

ORenderTargetBase* OAABBVisNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawAABBOfRenderItems(FindPSOInfo(PSO));
	return RenderTarget;
}