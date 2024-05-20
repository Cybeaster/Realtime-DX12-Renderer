#pragma once
#include "UI/Base/PickerTable/PickerTableWidget.h"
#include "UI/Widget.h"

struct SMaterial;
class OMaterialManager;
class OMaterialManagerWidget : public OPickerTableWidget
{
public:
	OMaterialManagerWidget(weak_ptr<OMaterialManager> InMaterialManager);

	void DrawTable() override;
	void DrawProperty() override;

private:
	SMaterial* CurrentMaterial = nullptr;
	weak_ptr<OMaterialManager> MaterialManager;

	float SplitterPercent = 0.25f;
	bool ResizingSplitter = false;
	const float SplitterWidth = 8.0f; // Width of the splitter
};