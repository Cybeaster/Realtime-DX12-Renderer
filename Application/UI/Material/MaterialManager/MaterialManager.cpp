
#include "MaterialManager.h"

#include "../../../../Materials/MaterialManager/MaterialManager.h"
#include "../../../../Utils/EngineHelper.h"
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

		/*const auto texture = FindTextureByPath(CurrentMaterial->TexturePath);
		const auto difTextureName = texture ? WStringToUTF8(texture->FileName) : string("No texture selected");
		ImGui::Text(difTextureName.c_str());
		if (texture)
		{
			auto srv = reinterpret_cast<void*>(OEngine::Get()->GetSRVDescHandleForTexture(texture).ptr);
			ImGui::Image(srv, ImVec2(100, 100));
		}*/
	}
	else
	{
		ImGui::Text("No material selected");
	}
}