//
// Created by Cybea on 12/02/2024.
//

#include "MaterialPicker.h"

#include "../../../Materials/MaterialManager/MaterialManager.h"

void OMaterialPickerWidget::Draw()
{
	auto name = string("Current material");
	name += CurrentMaterial ? ": " + CurrentMaterial->Name : ": None";
	ImGui::SeparatorText(name.c_str());
	if (ImGui::Button("Pick material"))
	{
		ImGui::OpenPopup("MaterialPicker");
	}
	if (ImGui::BeginPopup("MaterialPicker"))
	{
		ImGui::SeparatorText("Materials");

		float windowWidth = ImGui::GetContentRegionAvail().x;
		int numColumns = std::max(1, static_cast<int>(windowWidth / MinWidthPerColumn));
		auto& materials = MaterialManager->GetMaterials();
		ImGui::Columns(numColumns, nullptr, false);
		size_t counter = 0;
		for (auto& material : materials)
		{
			if (counter != 0 && counter % MaxMaterialsInColumn == 0)
			{
				ImGui::NextColumn();
			}

			if (ImGui::Selectable(material.first.c_str()))
			{
				CurrentMaterial = material.second.get();
				OnMaterialUpdate.Broadcast(CurrentMaterial);
			}
			counter++;
		}
		ImGui::Columns(1);
		ImGui::EndPopup();
	}
}