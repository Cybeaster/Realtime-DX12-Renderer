
#include "ShadowDebugNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Profiler.h"
#include "Window/Window.h"

void OShadowDebugNode::SetupCommonResources()
{
	PROFILE_SCOPE();
	auto resource = OEngine::Get()->CurrentFrameResource;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);
	auto ssao = OEngine::Get()->GetSSAORT();
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	//CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), PSO);
	//CommandQueue->SetResource("gShadowMaps", OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), PSO);

	CommandQueue->SetResource("gSsaoMap", OEngine::Get()->GetSSAORT()->GetAmbientMap0SRV().GPUHandle, pso);
	CommandQueue->SetResource("gNormalMap", OEngine::Get()->GetSSAORT()->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gDepthMap", ssao->GetDepthMapSRV().GPUHandle, pso);
}

ORenderTargetBase* OShadowDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();

	OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOTypes::ShadowDebug), SRenderLayers::ShadowDebug);
	return RenderTarget;
}
