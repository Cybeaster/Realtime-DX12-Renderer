
#include "DefaultRenderNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "EngineHelper.h"

void ODefaultRenderNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	OEngine::Get()->SetDescriptorHeap(EResourceHeapType::Default);

	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart(), PSO);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(), PSO);
	CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gShadowMaps", OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), PSO);
	CommandQueue->SetResource("gSsaoMap", OEngine::Get()->GetSSAORT()->GetAmbientMap0SRV().GPUHandle, PSO);
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