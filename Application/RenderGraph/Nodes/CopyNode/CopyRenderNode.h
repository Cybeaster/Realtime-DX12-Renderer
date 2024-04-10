
#pragma once
#include "RenderGraph/Nodes/RenderNode.h"
class OCopyRenderNode : public ORenderNode
{
public:
	OCopyRenderNode(ORenderTargetBase* OutTexture)
	    : OutTexture(OutTexture){};

	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;

private:
	ORenderTargetBase* OutTexture;
};
