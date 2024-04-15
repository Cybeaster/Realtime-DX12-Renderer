
#include "TangentNormalDebugNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"

void TangentNormalDebugNode::SetupCommonResources()
{
	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", OEngine::Get()->CurrentFrameResource->PassCB->GetGPUAddress(), PSO);
}

ORenderTargetBase* TangentNormalDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	auto normalTangentDebug = engine->GetNormalTangentDebugTarget();
	engine->DrawRenderItems(PSO, SRenderLayers::Opaque);
	return RenderTarget;
}
