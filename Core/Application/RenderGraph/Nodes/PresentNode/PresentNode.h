#pragma once
#include "RenderGraph/Nodes/RenderNode.h"
class OPresentNode : public ORenderNode
{
public:
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;

private:
};
