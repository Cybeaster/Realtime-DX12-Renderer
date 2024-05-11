
#pragma once
#include "../../../Materials/Material.h"
#include "UI/Widget.h"

class OMaterialManager;
class OMaterialPickerWidget : public IWidget
{
public:
	DECLARE_DELEGATE(SMaterialUpdate, weak_ptr<SMaterial>);
	OMaterialPickerWidget(OMaterialManager* _MaterialManager)
	    : MaterialManager(_MaterialManager) {}

	void Draw() override;

	SMaterialUpdate& GetOnMaterialUpdateDelegate() { return OnMaterialUpdate; }
	void SetCurrentMaterial(const weak_ptr<SMaterial>& Material) { CurrentMaterial = Material; }

private:
	SMaterialUpdate OnMaterialUpdate;
	uint32_t MaxMaterialsInColumn = 5;
	uint32_t MinWidthPerColumn = 100;
	weak_ptr<SMaterial> CurrentMaterial;
	OMaterialManager* MaterialManager = nullptr;
};
