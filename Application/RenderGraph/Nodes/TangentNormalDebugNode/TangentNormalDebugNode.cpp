
#include "TangentNormalDebugNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"

void TangentNormalDebugNode::SetupCommonResources()
{
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource("cbPass", OEngine::Get()->CurrentFrameResource->PassCB->GetGPUAddress(), pso);
}

ORenderTargetBase* TangentNormalDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	auto pso = FindPSOInfo(PSO);
	auto normalTangentDebug = engine->GetNormalTangentDebugTarget();
	engine->DrawRenderItems(pso, SRenderLayers::Opaque);
	return RenderTarget;
}
