#include "PickedRenderItem.h"

#include "Engine/Engine.h"
void OPickedRenderItemWidget::Draw()
{
	auto engine = OEngine::Get();
	auto picked = engine->GetPickedItem();
	ImGui::SeparatorText("Picked Item");
	if (picked)
	{
		ImGui::Text("Geometry Name: %s", !picked->Geometry.expired() ? picked->Geometry.lock()->Name.c_str() : "None");
		ImGui::Text("Material: %s", !picked->DefaultMaterial.expired() ? picked->DefaultMaterial.lock()->Name.c_str() : "None");
		ImGui::Text("Picked triangle %s", std::to_string(picked->ChosenSubmesh.lock()->StartIndexLocation).c_str());
	}
	else
	{
		ImGui::Text("Item not picked");
	}
}