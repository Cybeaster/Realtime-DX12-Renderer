#pragma once
#include "RenderGraph/Nodes/RenderNode.h"
class OPresentNode : public ORenderNode
{
public:
	virtual ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget);

private:
};
