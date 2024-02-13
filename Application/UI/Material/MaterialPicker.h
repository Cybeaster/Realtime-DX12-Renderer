
#pragma once
#include "../../../Materials/Material.h"
#include "UI/Widget.h"

class OMaterialManager;
class OMaterialPickerWidget : public IWidget
{
public:
	DECLARE_DELEGATE(SMaterialUpdate, SMaterial*);
	OMaterialPickerWidget(OMaterialManager* _MaterialManager)
	    : MaterialManager(_MaterialManager) {}

	void Draw() override;

	SMaterialUpdate& GetOnMaterialUpdateDelegate() { return OnMaterialUpdate; }

private:
	SMaterialUpdate OnMaterialUpdate;
	uint32_t MaxMaterialsInColumn = 10;
	uint32_t MinWidthPerColumn = 100;
	SMaterial* CurrentMaterial = nullptr;
	OMaterialManager* MaterialManager = nullptr;
};
