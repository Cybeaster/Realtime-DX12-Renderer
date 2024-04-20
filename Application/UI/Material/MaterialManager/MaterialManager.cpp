
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
		isMatChanged |= ImGui::InputFloat3("Diffuse", &CurrentMaterial->MaterialData.DiffuseAlbedo.x);
		isMatChanged |= ImGui::InputFloat3("Ambient", &CurrentMaterial->MaterialData.AmbientAlbedo.x);
		isMatChanged |= ImGui::InputFloat3("Specular", &CurrentMaterial->MaterialData.SpecularAlbedo.x);
		isMatChanged |= ImGui::InputFloat("Roughness", &CurrentMaterial->MaterialData.Roughness);
		isMatChanged |= ImGui::InputFloat("Metallic", &CurrentMaterial->MaterialData.Metalness);
		isMatChanged |= ImGui::InputFloat("Sheen", &CurrentMaterial->MaterialData.Sheen);
		isMatChanged |= ImGui::InputFloat("Shininess", &CurrentMaterial->MaterialData.Shininess);
		isMatChanged |= ImGui::InputFloat3("Transmittance", &CurrentMaterial->MaterialData.Transmittance.x);
		isMatChanged |= ImGui::InputFloat3("Emission", &CurrentMaterial->MaterialData.Emission.x);
		isMatChanged |= ImGui::InputFloat("IndexOfRefraction", &CurrentMaterial->MaterialData.IndexOfRefraction);
		isMatChanged |= ImGui::InputInt("Illumination", &CurrentMaterial->MaterialData.Illumination);
		isMatChanged |= ImGui::InputFloat("Dissolve", &CurrentMaterial->MaterialData.Dissolve);

		if (isMatChanged)
		{
			MaterialManager->OnMaterialChanged(CurrentMaterial->Name);
		}
		ImGui::SeparatorText("Diffuse Texture");
		const auto& diffuse = CurrentMaterial->DiffuseMap;
		if (diffuse.IsValid())
		{
			const auto difTextureName = diffuse.Texture ? WStringToUTF8(diffuse.Texture->FileName) : string("No texture selected");
			ImGui::Text(difTextureName.c_str());
			if (diffuse.Texture)
			{
				auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(diffuse.Texture).ptr);
				ImGui::Image(srv, ImVec2(100, 100));
			}
		}
		else
		{
			ImGui::Text("No diffuse map");
		}

		ImGui::NewLine();

		ImGui::SeparatorText("Normal Texture");
		const auto& normMap = CurrentMaterial->NormalMap;
		if (!normMap.IsValid())
		{
			ImGui::Text("No normal map");
		}
		else
		{
			const auto normMapName = WStringToUTF8(CurrentMaterial->NormalMap.Texture->FileName);
			ImGui::Text(normMapName.c_str());
			auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(normMap.Texture).ptr);
			ImGui::Image(srv, ImVec2(100, 100));

			ImGui::NewLine();
		}

		ImGui::SeparatorText("Height Texture");
		const auto& heightMap = CurrentMaterial->HeightMap;
		if (heightMap.IsValid())
		{
			const auto heighTexName = WStringToUTF8(CurrentMaterial->HeightMap.Texture->FileName);
			ImGui::Text(heighTexName.c_str());
			if (heightMap.Texture)
			{
				auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(heightMap.Texture).ptr);
				ImGui::Image(srv, ImVec2(100, 100));
			}

			ImGui::NewLine();
		}
		else
		{
			ImGui::Text("No height map");
		}

		ImGui::SeparatorText("Alpha Texture");
		const auto alphaTexName = CurrentMaterial->AlphaMap.Texture ? WStringToUTF8(CurrentMaterial->AlphaMap.Texture->FileName) : string("No texture selected");
		ImGui::Text(alphaTexName.c_str());
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