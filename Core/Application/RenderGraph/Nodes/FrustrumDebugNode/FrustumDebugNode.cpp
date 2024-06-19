//
// Created by Cybea on 29/04/2024.
//

#include "FrustumDebugNode.h"

#include "Engine/Engine.h"
#include "Window/Window.h"
ORenderTargetBase* OFrustumDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(FindPSOInfo(PSO), GetNodeInfo().RenderLayer);
	return RenderTarget;
}
void OFrustumDebugNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), resource->CameraBuffer->GetGPUAddress(), pso);
}