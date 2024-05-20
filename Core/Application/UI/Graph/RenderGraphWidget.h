

#pragma once
#include "UI/Widget.h"

class OOpaquePassNodeWidget;
class ORenderNodeWidgetBase;
class OSSAORenderNodeWidget;
class ORenderNode;
class ORenderGraph;
class ORenderGraphWidget : public OHierarchicalWidgetBase
{
public:
	explicit ORenderGraphWidget(ORenderGraph* InGraph);

	void Draw() override;
	void InitWidget() override;

public:
	ORenderGraph* Graph = nullptr;
	ORenderNode* Node = nullptr;
	OSSAORenderNodeWidget* SSAONodeWidget = nullptr;
	ORenderNodeWidgetBase* BaseNodeWidget = nullptr;
	OOpaquePassNodeWidget* OpaquePassNodeWidget = nullptr;
};
