#pragma once
#include "UI/Base/PickerTable/PickerTableWidget.h"
#include "UI/Widget.h"

struct STexture;
class OTextureManager;
class OTextureManagerWidget final : public OPickerTableWidget
{
public:
	OTextureManagerWidget(weak_ptr<OTextureManager> Other);
	void Draw() override;

public:
	void DrawTable() override;
	void DrawProperty() override;

private:
	weak_ptr<OTextureManager> TextureManager;
	STexture* CurrentTexture = nullptr;
};
