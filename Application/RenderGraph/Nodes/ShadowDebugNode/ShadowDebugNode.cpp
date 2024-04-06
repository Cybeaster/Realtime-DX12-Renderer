
#include "ShadowDebugNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Window/Window.h"

void OShadowDebugNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);
	auto ssao = OEngine::Get()->GetSSAORT();

	CommandQueue->SetPipelineState(PSO);
	//CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gShadowMaps", OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), PSO);

	CommandQueue->SetResource("gSsaoMap", OEngine::Get()->GetSSAORT()->GetAmbientMap0SRV().GPUHandle, PSO);
	CommandQueue->SetResource("gNormalMap", OEngine::Get()->GetSSAORT()->GetNormalMapSRV().GPUHandle, PSO);
	CommandQueue->SetResource("gDepthMap", ssao->GetDepthMapSRV().GPUHandle, PSO);
}

ORenderTargetBase* OShadowDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOType::ShadowDebug), SRenderLayer::ShadowDebug);
	return RenderTarget;
}
