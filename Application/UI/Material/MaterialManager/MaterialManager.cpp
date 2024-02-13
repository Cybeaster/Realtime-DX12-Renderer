//
// Created by Cybea on 13/02/2024.
//

#include "MaterialManager.h"

#include "../../../../Materials/MaterialManager/MaterialManager.h"
void OMaterialManagerWidget::Draw()
{
	if (ImGui::CollapsingHeader("Material Manager"))
	{
		if (ImGui::Button("Load Materials"))
		{
			MaterialManager->LoadMaterials();
		}
		if (ImGui::Button("Save Materials"))
		{
			MaterialManager->SaveMaterials();
		}
		if (ImGui::TreeNode("Materials"))
		{
			for (auto& material : MaterialManager->GetMaterials())
			{
				if (ImGui::Button(material.first.c_str()))
				{
					CurrentMaterial = material.second.get();
				}
			}
			ImGui::TreePop();
		}
	}
}