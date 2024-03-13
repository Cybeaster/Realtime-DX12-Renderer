#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class OUIRenderNode : public ORenderNode
{
public:
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
	void SetupCommonResources() override;
};
