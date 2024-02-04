#include "GeometryManager.h"

#include <ranges>

void OGeometryManagerWidget::Draw()
{
	ImGui::SeparatorText("Geometry manager");
	ImGui::Text("Number of geometries %d", RenderLayers->size());
	OGeometryEntityWidget* selectedWidget = nullptr;

	if (ImGui::TreeNode("Geometries"))
	{
		for (auto& entity : GetWidgets())
		{
			if (const auto widget = SCast<OGeometryEntityWidget>(entity.get()))
			{
				const auto geometry = widget->GetGeometry();
				if (ImGui::Selectable(geometry->Name.c_str(), SelectedGeometry == geometry->Name))
				{
					SelectedGeometry = geometry->Name;
				}

				if (SelectedGeometry == geometry->Name)
				{
					selectedWidget = widget;
				}
			}
		}
		ImGui::TreePop();
	}

	if (selectedWidget)
	{
		selectedWidget->Draw();
	}
}

void OGeometryManagerWidget::Update()
{
	OHierarchicalWidgetBase::Update();
	if (bRebuildRequested)
	{
		Engine->RebuildGeometry(SelectedGeometry);
		bRebuildRequested = false;
	}
}

void OGeometryManagerWidget::Init()
{
	IWidget::Init();

	for (auto& layer : *RenderLayers | std::views::values)
	{
		for (auto item : layer)
		{
			MakeWidget<OGeometryEntityWidget>(item, Engine, this);
		}
	}
}

void OGeometryManagerWidget::RebuildRequest()
{
	bRebuildRequested = true;
}