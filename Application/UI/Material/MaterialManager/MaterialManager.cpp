
#include "MaterialManager.h"

#include "EngineHelper.h"
#include "MaterialManager/MaterialManager.h"
#include "imgui_internal.h"

OMaterialManagerWidget::OMaterialManagerWidget(OMaterialManager* _MaterialManager)
    : MaterialManager(_MaterialManager)
{
	HeadderName = "Material Manager";
	PropertyName = "Material Properties";
	ListName = "Material List";
}

void OMaterialManagerWidget::DrawTable()
{
	for (auto& material : MaterialManager->GetMaterials())
	{
		if (ImGui::Selectable(material.first.c_str()))
		{
			CurrentMaterial = material.second.get();
		}
	}
}

void OMaterialManagerWidget::DrawProperty()
{
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
		for (auto& path : CurrentMaterial->DiffuseMaps)
		{
			const auto difTextureName = path.Texture ? WStringToUTF8(path.Texture->FileName) : string("No texture selected");
			ImGui::Text(difTextureName.c_str());
			if (path.Texture)
			{
				auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(path.Texture).ptr);
				ImGui::Image(srv, ImVec2(100, 100));
			}
			ImGui::SameLine();
		}
		ImGui::NewLine();

		ImGui::SeparatorText("Normal Texture");
		for (auto& path : CurrentMaterial->NormalMaps)
		{
			const auto difTextureName = path.Texture ? WStringToUTF8(path.Texture->FileName) : string("No texture selected");
			ImGui::Text(difTextureName.c_str());
			if (path.Texture)
			{
				auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(path.Texture).ptr);
				ImGui::Image(srv, ImVec2(100, 100));
			}
			ImGui::SameLine();
		}
		ImGui::NewLine();

		for (auto& path : CurrentMaterial->HeightMaps)
		{
			const auto difTextureName = path.Texture ? WStringToUTF8(path.Texture->FileName) : string("No texture selected");
			ImGui::Text(difTextureName.c_str());
			if (path.Texture)
			{
				auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(path.Texture).ptr);
				ImGui::Image(srv, ImVec2(100, 100));
			}
			ImGui::SameLine();
		}
	}
	else
	{
		ImGui::Text("No material selected");
	}
}