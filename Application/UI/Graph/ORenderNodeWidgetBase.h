
#pragma once
#include "UI/Widget.h"

class ORenderNode;
class ORenderNodeWidgetBase : public IWidget
{
public:
	ORenderNodeWidgetBase() = default;
	void SetNode(ORenderNode* InNode) { Node = InNode; }
	void Draw() override;

protected:
	ORenderNode* Node = nullptr;
};
