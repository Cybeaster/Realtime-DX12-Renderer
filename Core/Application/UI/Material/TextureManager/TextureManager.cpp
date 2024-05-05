//
// Created by Cybea on 24/02/2024.
//

#include "TextureManager.h"

#include "TextureManager/TextureManager.h"

#include <ranges>
OTextureManagerWidget::OTextureManagerWidget(OTextureManager* Other)
    : TextureManager(Other)
{
	HeadderName = "Textures Manager";
	ListName = "Textures List";
	PropertyName = "Texture Properties";
}

void OTextureManagerWidget::Draw()
{
	OPickerTableWidget::Draw();
}

void OTextureManagerWidget::DrawTable()
{
	for (auto& val : TextureManager->GetTextures() | std::views::values)
	{
		if (ImGui::Selectable(val->Name.c_str()))
		{
			CurrentTexture = val.get();
		}
	}
}

void OTextureManagerWidget::DrawProperty()
{
	if (CurrentTexture)
	{
		ImGui::Text(CurrentTexture->Name.c_str());
		ImGui::Text(WStringToUTF8(CurrentTexture->FileName).c_str());
		ImGui::Text("Heap Index: %d", CurrentTexture->HeapIdx);
		if (ImGui::BeginCombo("##textureCombo", CurrentTexture->ViewType.c_str()))
		{
			for (const auto& type : STextureViewType::GetTextureTypes())
			{
				bool isSelected = CurrentTexture->ViewType == type;
				if (ImGui::Selectable(type.c_str(), isSelected))
				{
					CurrentTexture->ViewType = type;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		ImGui::Text("No texture selected");
	}
}