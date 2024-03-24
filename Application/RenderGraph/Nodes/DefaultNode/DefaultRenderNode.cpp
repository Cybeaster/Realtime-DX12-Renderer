
#include "DefaultRenderNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"

void ODefaultRenderNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResources;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);

	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart(), PSO);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(), PSO);
	CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), PSO);

	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Shadow);
	CommandQueue->SetResource("gShadowMaps", OEngine::Get()->ShadowHeap.SRVHeap->GetGPUDescriptorHandleForHeapStart(), PSO);
}

void ODefaultRenderNode::Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue,
                                    ORenderGraph* OtherParentGraph, SPSODescriptionBase* OtherPSO)
{
	ORenderNode::Initialize(OtherNodeInfo, OtherCommandQueue, OtherParentGraph, OtherPSO);
}

ORenderTargetBase* ODefaultRenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(PSO, GetNodeInfo().RenderLayer);
	return RenderTarget;
}