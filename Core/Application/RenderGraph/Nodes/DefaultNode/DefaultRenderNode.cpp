
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
	CommandQueue->SetResource(STRINGIFY_MACRO(MATERIAL_DATA), resource->MaterialBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(TEXTURE_MAPS), OEngine::Get()->TexturesStartAddress.GPUHandle, pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CUBE_MAP), GetSkyTextureSRV(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(DIRECTIONAL_LIGHTS), resource->DirectionalLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(POINT_LIGHTS), resource->PointLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(SPOT_LIGHTS), resource->SpotLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(SHADOW_MAPS), OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(SSAO_MAP), OEngine::Get()->GetSSAORT().lock()->GetAmbientMap0SRV().GPUHandle, pso);
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