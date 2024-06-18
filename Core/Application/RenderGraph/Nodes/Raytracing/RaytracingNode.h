#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class ORaytracer;
class ORaytracingNode : public ORenderNode
{
public:
	ORaytracingNode(const shared_ptr<ORaytracer>& InRaytracer)
	    : Raytracer(InRaytracer) {}

	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
	void SetupCommonResources() override;

private:
	shared_ptr<ORaytracer> Raytracer;
};
