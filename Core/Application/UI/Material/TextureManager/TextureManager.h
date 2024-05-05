#pragma once
#include "UI/Base/PickerTable/PickerTableWidget.h"
#include "UI/Widget.h"

struct STexture;
class OTextureManager;
class OTextureManagerWidget final : public OPickerTableWidget
{
public:
	OTextureManagerWidget(OTextureManager* Other);
	void Draw() override;

public:
	void DrawTable() override;
	void DrawProperty() override;

private:
	OTextureManager* TextureManager = nullptr;
	STexture* CurrentTexture = nullptr;
};
