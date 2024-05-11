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
					const auto ri = widget->GetRenderItem();
					if (!ri.expired())
					{
						auto lock = ri.lock();
						if (lock->bIsDisplayable == false)
						{
							continue;
						}

						if (ImGui::Selectable(lock->Name.c_str(), SelectedRenderItem == lock->Name))
						{
							SelectedRenderItem = lock->Name;
							SelectedComponentName = "";
							SelectedComponent = nullptr;
						}

						if (SelectedRenderItem == lock->Name)
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
		for (auto item : layer.second)
		{
			auto lock = item.lock();
			if (!lock->Geometry.expired())
			{
				MakeWidget<OGeometryEntityWidget>(item, Engine, this);
				StringToGeo[lock->Name] = item;
			}
		}
	}
	LightComponentWidget = MakeWidget<OLightComponentWidget>(); //TODO: move to its own widget
}

void OGeometryManagerWidget::RebuildRequest() const
{
	Engine->UpdateGeometryRequest(SelectedRenderItem);
}

void OGeometryManagerWidget::DrawComponentWidgets()
{
	ImGui::SeparatorText("Item components");
	auto item = StringToGeo[SelectedRenderItem];
	if (!item.expired())
	{
		auto lock = item.lock();
		auto compNum = lock->GetComponents().size();
		ImGui::Text("Number of components %zu", compNum);
		if (compNum > 0)
		{
			if (ImGui::TreeNode("Components"))
			{
				uint32_t it = 0;
				for (auto& comp : lock->GetComponents())
				{
					auto name = comp->GetName();
					name += "##" + std::to_string(it);
					if (ImGui::Selectable(name.c_str()))
					{
						SelectedComponentName = comp->GetName();
						SelectedComponent = comp.get();
					}
					it++;
				}
				ImGui::TreePop();
			}
			if (SelectedComponentName != "")
			{
				if (SelectedComponent)
				{
					if (auto casted = Cast<OLightComponent>(SelectedComponent))
					{
						LightComponentWidget->SetComponent(casted);
						LightComponentWidget->Draw();
					}
				}
			}
		}
	}
}