#include "RenderNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"

ORenderTargetBase* ORenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	return RenderTarget;
}

void ORenderNode::SetupCommonResources()
{
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
