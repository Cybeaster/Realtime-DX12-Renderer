#pragma once
#include "RenderGraph/Nodes/DefaultNode/DefaultRenderNode.h"
#include "RenderGraph/Nodes/RenderNode.h"

class OReflectionNode : public ODefaultRenderNode
{
public:
	void SetupCommonResources() override;
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
};
