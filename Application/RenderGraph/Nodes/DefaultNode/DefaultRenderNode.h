
#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class ODefaultRenderNode : public ORenderNode
{
public:
	void SetupCommonResources() override;
	void Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue, ORenderGraph* OtherParentGraph, SPSODescriptionBase* OtherPSO) override;
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
