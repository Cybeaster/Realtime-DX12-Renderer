//
// Created by Cybea on 10/03/2024.
//

#include "DefaultRenderNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"
void ODefaultRenderNode::SetupCommonResources()
{
	ORenderNode::SetupCommonResources();
	auto resource = OEngine::Get()->CurrentFrameResources;
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList().Get();
	CommandQueue->SetPipelineState(PSO);
	PSO->RootSignature->SetResourceCBView("cbPass", resource->PassCB->GetGPUAddress(), cmdList);
	PSO->RootSignature->SetResourceCBView("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), cmdList);
	PSO->RootSignature->SetDescriptorTable("gTextureMaps", OEngine::Get()->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart(), cmdList);
	PSO->RootSignature->SetDescriptorTable("gCubeMap", GetSkyTextureSRV(), cmdList);
}
ORenderTargetBase* ODefaultRenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	OEngine::Get()->DrawRenderItems(PSO->RootSignature.get(), GetNodeInfo().RenderLayer);
	return RenderTarget;
}