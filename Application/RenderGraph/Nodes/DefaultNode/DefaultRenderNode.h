
#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class ODefaultRenderNode : public ORenderNode
{
	void SetupCommonResources() override;
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
