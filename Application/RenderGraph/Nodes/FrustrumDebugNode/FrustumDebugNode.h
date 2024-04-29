#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class OFrustumDebugNode : public ORenderNode
{
public:
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
