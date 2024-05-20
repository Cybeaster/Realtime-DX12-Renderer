
#pragma once
#include "RenderGraph/Nodes/RenderNode.h"
class OCopyRenderNode : public ORenderNode
{
public:
	OCopyRenderNode(const shared_ptr<ORenderTargetBase>& OutTexture)
	    : OutTexture(OutTexture){};

	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;

private:
	weak_ptr<ORenderTargetBase> OutTexture;
};
