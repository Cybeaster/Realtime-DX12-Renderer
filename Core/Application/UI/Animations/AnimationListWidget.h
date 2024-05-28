#pragma once
#include "UI/Base/PickerTable/PickerTableWidget.h"
#include "UI/Widget.h"

class OAnimation;
class OAnimationManager;
class OAnimationListWidget : public OPickerTableWidget
{
public:
	OAnimationListWidget(const shared_ptr<OAnimationManager>& InAnimationManager)
	    : AnimationManager(InAnimationManager)
	{
		HeadderName = "Animations Manager";
		PropertyName = "Animations Properties";
		ListName = "Animations List";
	}

	void DrawTable() override;
	void DrawProperty() override;

private:
	weak_ptr<OAnimationManager> AnimationManager;
	weak_ptr<OAnimation> CurrentAnimation;
};
