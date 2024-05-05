#pragma once
#include "RenderGraph/Nodes/DefaultNode/DefaultRenderNode.h"
class OShadowMapNode : public ODefaultRenderNode
{
public:
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
	void SetupCommonResources() override;
};

