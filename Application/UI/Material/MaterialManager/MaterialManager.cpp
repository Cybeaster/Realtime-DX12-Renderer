//
// Created by Cybea on 13/02/2024.
//

#include "MaterialManager.h"

#include "../../../../Materials/MaterialManager/MaterialManager.h"
#include "../../../../Utils/EngineHelper.h"
#include "imgui_internal.h"
void OMaterialManagerWidget::Draw()
{
	if (ImGui::CollapsingHeader("Material Manager"))
	{
		// Calculate the width for the two child windows
		float widthAvailable = ImGui::GetContentRegionAvail().x - SplitterWidth;
		float width1 = widthAvailable * SplitterPercent;
		float width2 = widthAvailable - width1;

		// First child window (left side with material list)
		ImGui::BeginChild("MaterialList", ImVec2(width1, 0), true);
		for (auto& material : MaterialManager->GetMaterials())
		{
			if (ImGui::Selectable(material.first.c_str()))
			{
				CurrentMaterial = material.second.get();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Splitter logic
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (SplitterWidth * 0.5f));
		ImGui::Button("##Splitter", ImVec2(SplitterWidth, -1));
		if (ImGui::IsItemActive())
		{
			float deltaX = ImGui::GetIO().MouseDelta.x;
			SplitterPercent += deltaX / widthAvailable;
			SplitterPercent = ImClamp(SplitterPercent, 0.1f, 0.9f); // Ensure splitter stays within bounds
			ResizingSplitter = true;
		}
		ImGui::SameLine();

		// Second child window (right side with material properties editor)
		ImGui::BeginChild("MaterialProperties", ImVec2(width2, 0), true);
		if (CurrentMaterial)
		{
			bool isMatChanged = false;
			ImGui::SeparatorText("Material Surfaces");
			isMatChanged |= ImGui::InputFloat4("Diffuse", &CurrentMaterial->MaterialSurface.DiffuseAlbedo.x);
			isMatChanged |= ImGui::InputFloat3("Fesnel", &CurrentMaterial->MaterialSurface.FresnelR0.x);
			isMatChanged |= ImGui::InputFloat("Roughness", &CurrentMaterial->MaterialSurface.Roughness);
			if (isMatChanged)
			{
				MaterialManager->OnMaterialChanged(CurrentMaterial->Name);
			}
			ImGui::SeparatorText("Diffuse Texture");
			const auto texture = FindTextureByPath(CurrentMaterial->TexturePath);
			const auto difTextureName = texture ? WStringToUTF8(texture->FileName) : string("No texture selected");
			ImGui::Text(difTextureName.c_str());
			if (texture)
			{
				auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(texture).ptr);
				ImGui::Image(srv, ImVec2(100, 100));
			}
		}
		else
		{
			ImGui::Text("No material selected");
		}
		ImGui::EndChild();
	}
}