#pragma once
#include "UI/Widget.h"

class OTextureManager;
class OTextureManagerWidget final : public OHierarchicalWidgetBase
{
	OTextureManagerWidget(OTextureManager* Other)
	    : TextureManager(Other){};
	void Draw() override;

private:
	OTextureManager* TextureManager = nullptr;
};
