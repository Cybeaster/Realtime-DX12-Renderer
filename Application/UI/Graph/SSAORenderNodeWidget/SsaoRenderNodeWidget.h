
#pragma once
#include "UI/Graph/ORenderNodeWidgetBase.h"
#include "UI/Widget.h"

class ORenderNode;
class OSSAONode;
class OSSAORenderNodeWidget : public ORenderNodeWidgetBase
{
public:
	OSSAORenderNodeWidget() = default;
	void Draw() override;
	OSSAONode* GetRenderNode() const;
};
