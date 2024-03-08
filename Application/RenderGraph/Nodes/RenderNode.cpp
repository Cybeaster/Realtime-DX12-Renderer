#include "RenderNode.h"

#include "../../../Utils/EngineHelper.h"
#include "Engine/Engine.h"

ORenderTargetBase* ORenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	RenderTarget->PrepareRenderTarget(CommandQueue);
	OEngine::Get()->DrawRenderItems(PSO->PSO.Get(), NodeInfo.RenderLayer);
	return RenderTarget;
}

void ORenderNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResources;
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList().Get();
	auto passCB = resource->PassCB->GetResource();

	PSO->RootSignature->SetResourceCBView("PassCB", passCB->GetGPUVirtualAddress(), cmdList);
	PSO->RootSignature->SetResourceCBView("MaterialCB", resource->MaterialBuffer->GetResource()->GetGPUVirtualAddress(), cmdList);
	PSO->RootSignature->SetDescriptorTable("gTextureMaps", OEngine::Get()->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart(), cmdList);
	PSO->RootSignature->SetDescriptorTable("gCubeMap", GetSkyTextureSRV(), cmdList);
}

void ORenderNode::SetPSO(const string& PSOType) const
{
	ParentGraph->SetPSO(PSOType);
}

SPSODescriptionBase* ORenderNode::FindPSOInfo(string Name) const
{
	return ParentGraph->FindPSOInfo(Name);
}

void ORenderNode::Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue, ORenderGraph* OtherParentGraph, SPSODescriptionBase* OtherPSO)
{
	NodeInfo = OtherNodeInfo;
	CommandQueue = OtherCommandQueue;
	PSO = OtherPSO;
	ParentGraph = OtherParentGraph;
}
