#include "GeometryManager.h"

#include "GeometryEntity/PickedRenderItem.h"

#include <ranges>

void OGeometryManagerWidget::Draw()
{
	if (ImGui::CollapsingHeader("Geometry Manager"))
	{
		PickedRenderItemWidget->Draw();
		ImGui::Text("Number of geometries %d", RenderLayers->size());
		OGeometryEntityWidget* selectedWidget = nullptr;

		if (ImGui::TreeNode("Geometries"))
		{
			for (auto& entity : GetWidgets())
			{
				if (const auto widget = Cast<OGeometryEntityWidget>(entity.get()))
				{
					if (const auto ri = widget->GetRenderItem())
					{
						if (ImGui::Selectable(ri->Name.c_str(), SelectedRenderItem == ri->Name))
						{
							SelectedRenderItem = ri->Name;
						}

						if (SelectedRenderItem == ri->Name)
						{
							selectedWidget = widget;
						}
					}
				}
			}
			ImGui::TreePop();
		}

		if (selectedWidget)
		{
			selectedWidget->Draw();
			DrawComponentWidgets();
		}
	}
}

void OGeometryManagerWidget::InitWidget()
{
	IWidget::InitWidget();
	PickedRenderItemWidget = MakeWidget<OPickedRenderItemWidget>();
	for (auto& layer : *RenderLayers)
	{
		if (layer.first == "Opaque" || layer.first == "Transparent" || layer.first == "Sky" || layer.first == "Water")
		{
			for (auto item : layer.second)
			{
				if (item->Geometry)
				{
					MakeWidget<OGeometryEntityWidget>(item, Engine, this);
					StringToGeo[item->Name] = item;
				}
			}
		}
	}
	LightComponentWidget = MakeWidget<OLightComponentWidget>();
}

void OGeometryManagerWidget::RebuildRequest() const
{
	Engine->UpdateGeometryRequest(SelectedRenderItem);
}

void OGeometryManagerWidget::DrawComponentWidgets()
{
	ImGui::SeparatorText("Item components");
	auto item = StringToGeo[SelectedRenderItem];
	if(item)
	{
		auto compNum = item->GetComponents().size();
		ImGui::Text("Number of components %zu", compNum);
		if(compNum> 0)
		{
			if(ImGui::TreeNode("Components"))
			{
				uint32_t it = 0;
				for (auto& comp : item->GetComponents())
				{
					auto name = comp->GetName();
					name += "##" + std::to_string(it);
					if(ImGui::Selectable(name.c_str()))
					{
						SelectedComponentName = comp->GetName();
						SelectedComponent = comp.get();
					}
					it++;
				}
				ImGui::TreePop();

			}
			if(SelectedComponentName != "")
			{
				if(SelectedComponent)
				{
					if(auto casted = Cast<OLightComponent>(SelectedComponent))
					{
						LightComponentWidget->SetComponent(casted);
						LightComponentWidget->Draw();
					}

				}
			}
		}


	}
}