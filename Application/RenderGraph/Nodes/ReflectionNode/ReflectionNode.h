#pragma once
#include "RenderGraph/Nodes/DefaultNode/DefaultRenderNode.h"
#include "RenderGraph/Nodes/RenderNode.h"

class OReflectionNode : public ODefaultRenderNode
{
public:
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
