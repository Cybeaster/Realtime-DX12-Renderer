
#include "DefaultRenderNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "EngineHelper.h"
#include "Profiler.h"

void ODefaultRenderNode::SetupCommonResources()
{
	PROFILE_SCOPE();

	auto resource = OEngine::Get()->CurrentFrameResource;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);
	auto pso = FindPSOInfo(PSO);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), resource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart(), pso);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(), pso);
	CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource("gShadowMaps", OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), pso);
	CommandQueue->SetResource("gSsaoMap", OEngine::Get()->GetSSAORT()->GetAmbientMap0SRV().GPUHandle, pso);
}

void ODefaultRenderNode::Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue,
                                    ORenderGraph* OtherParentGraph, const SPSOType& Type)
{
	ORenderNode::Initialize(OtherNodeInfo, OtherCommandQueue, OtherParentGraph, Type);
}

ORenderTargetBase* ODefaultRenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	OEngine::Get()->DrawRenderItems(FindPSOInfo(PSO), GetNodeInfo().RenderLayer);
	return RenderTarget;
}