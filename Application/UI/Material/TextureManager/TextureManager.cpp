//
// Created by Cybea on 24/02/2024.
//

#include "TextureManager.h"

#include "../../../../Textures/TextureManager/TextureManager.h"
void OTextureManagerWidget::Draw()
{
	if (ImGui::CollapsingHeader("Textures"))
	{
		if (ImGui::Button("Save"))
		{
			TextureManager->SaveLocalTextures();
		}
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			TextureManager->LoadLocalTextures();
		}
	}
}