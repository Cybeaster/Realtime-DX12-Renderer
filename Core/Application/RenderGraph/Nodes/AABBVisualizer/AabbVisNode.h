#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class OAABBVisNode : public ORenderNode
{
public:
	void SetupCommonResources() override;
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
