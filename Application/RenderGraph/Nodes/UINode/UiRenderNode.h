#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class OUIRenderNode : public ORenderNode
{
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
	void SetupCommonResources() override;
};
