
#include "ShadowDebugNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Profiler.h"
#include "Window/Window.h"

void OShadowDebugNode::SetupCommonResources()
{
	PROFILE_SCOPE();
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	auto ssao = OEngine::Get()->GetSSAORT().lock();
	CommandQueue->SetResource("gShadowMaps", OEngine::Get()->GetRenderGroupStartAddress(ShadowTextures), pso);
	CommandQueue->SetResource("gSsaoMap", ssao->GetAmbientMap0SRV().GPUHandle, pso);
	CommandQueue->SetResource("gNormalMap", ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gDepthMap", ssao->GetDepthMapSRV().GPUHandle, pso);
}

ORenderTargetBase* OShadowDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();

	OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOTypes::ShadowDebug), SRenderLayers::ShadowDebug);
	return RenderTarget;
}
