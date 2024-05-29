#pragma once
#include "UI/Base/PickerTable/PickerTableWidget.h"
#include "UI/Widget.h"

class OAnimation;
class OAnimationManager;
class OAnimationListWidget : public OPickerTableWidget
{
public:
	OAnimationListWidget(OAnimationManager* InAnimationManager)
	    : AnimationManager(InAnimationManager)
	{
		HeadderName = "Animations Manager";
		PropertyName = "Animations Properties";
		ListName = "Animations List";
	}

	void DrawTable() override;
	void DrawProperty() override;

private:
	OAnimationManager* AnimationManager;
	weak_ptr<OAnimation> CurrentAnimation;
};
