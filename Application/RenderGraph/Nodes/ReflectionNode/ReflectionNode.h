#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class OReflectionNode : public ORenderNode
{
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
