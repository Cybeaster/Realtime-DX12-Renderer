

#pragma once
#include "UI/Widget.h"

class ORenderNode;
class ORenderGraph;
class ORenderGraphWidget : public OHierarchicalWidgetBase
{
public:
	explicit ORenderGraphWidget(ORenderGraph* InGraph);

	void Draw() override;

public:
	ORenderGraph* Graph = nullptr;

	ORenderNode* Node = nullptr;
};
