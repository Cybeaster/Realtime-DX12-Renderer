
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
		isMatChanged |= ImGui::InputFloat3("Diffuse", &CurrentMaterial->MaterialSurface.DiffuseAlbedo.x);
		isMatChanged |= ImGui::InputFloat3("Ambient", &CurrentMaterial->MaterialSurface.AmbientAlbedo.x);
		isMatChanged |= ImGui::InputFloat3("Specular", &CurrentMaterial->MaterialSurface.SpecularAlbedo.x);
		isMatChanged |= ImGui::InputFloat("Roughness", &CurrentMaterial->MaterialSurface.Roughness);
		isMatChanged |= ImGui::InputFloat("Metallic", &CurrentMaterial->MaterialSurface.Metallic);
		isMatChanged |= ImGui::InputFloat("Sheen", &CurrentMaterial->MaterialSurface.Sheen);
		isMatChanged |= ImGui::InputFloat("Shininess", &CurrentMaterial->MaterialSurface.Shininess);
		isMatChanged |= ImGui::InputFloat3("Transmittance", &CurrentMaterial->MaterialSurface.Transmittance.x);
		isMatChanged |= ImGui::InputFloat3("Emission", &CurrentMaterial->MaterialSurface.Emission.x);
		isMatChanged |= ImGui::InputFloat("IndexOfRefraction", &CurrentMaterial->MaterialSurface.IndexOfRefraction);
		isMatChanged |= ImGui::InputInt("Illumination", &CurrentMaterial->MaterialSurface.Illumination);
		isMatChanged |= ImGui::InputFloat("Dissolve", &CurrentMaterial->MaterialSurface.Dissolve);

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
		if (CurrentMaterial->NormalMaps.empty())
		{
			ImGui::Text("No normal map");
		}
		else
		{
			for (auto& path : CurrentMaterial->NormalMaps)
			{
				if (path.Texture)
				{
					const auto difTextureName = WStringToUTF8(path.Texture->FileName);
					ImGui::Text(difTextureName.c_str());
					if (path.Texture)
					{
						auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(path.Texture).ptr);
						ImGui::Image(srv, ImVec2(100, 100));
					}
					ImGui::SameLine();
				}
			}
			ImGui::NewLine();
		}

		ImGui::SeparatorText("Height Texture");
		if (!CurrentMaterial->HeightMaps.empty())
		{
			for (auto& path : CurrentMaterial->HeightMaps)
			{
				if (path.Texture)
				{
					const auto difTextureName = WStringToUTF8(path.Texture->FileName);
					ImGui::Text(difTextureName.c_str());
					if (path.Texture)
					{
						auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(path.Texture).ptr);
						ImGui::Image(srv, ImVec2(100, 100));
					}
					ImGui::SameLine();
				}
			}
			ImGui::NewLine();
		}
		else
		{
			ImGui::Text("No height map");
		}

		ImGui::SeparatorText("Alpha Texture");
		const auto difTextureName = CurrentMaterial->AlphaMap.Texture ? WStringToUTF8(CurrentMaterial->AlphaMap.Texture->FileName) : string("No texture selected");
		ImGui::Text(difTextureName.c_str());
		if (CurrentMaterial->AlphaMap.Texture)
		{
			auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(CurrentMaterial->AlphaMap.Texture).ptr);
			ImGui::Image(srv, ImVec2(100, 100));
		}
	}
	else
	{
		ImGui::Text("No material selected");
	}
}