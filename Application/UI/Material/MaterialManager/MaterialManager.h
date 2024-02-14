#pragma once
#include "UI/Widget.h"

struct SMaterial;
class OMaterialManager;
class OMaterialManagerWidget : public OHierarchicalWidgetBase
{
public:
	OMaterialManagerWidget(OMaterialManager* _MaterialManager)
	    : MaterialManager(_MaterialManager) {}

	void Draw() override;

private:
	SMaterial* CurrentMaterial = nullptr;
	OMaterialManager* MaterialManager = nullptr;

	float SplitterPercent = 0.25f;
	bool ResizingSplitter = false;
	const float SplitterWidth = 8.0f; // Width of the splitter
};