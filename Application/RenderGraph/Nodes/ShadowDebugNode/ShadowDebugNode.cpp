
#include "ShadowDebugNode.h"

#include "Engine/Engine.h"

void OShadowDebugNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResources;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);

	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gShadowMaps", OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), PSO);
}

ORenderTargetBase* OShadowDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOType::ShadowDebug), SRenderLayer::ShadowDebug);
	return RenderTarget;
}
