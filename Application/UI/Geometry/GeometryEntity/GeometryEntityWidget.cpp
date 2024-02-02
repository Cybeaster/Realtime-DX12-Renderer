//
// Created by Cybea on 02/02/2024.
//

#include "GeometryEntityWidget.h"

#include "DXHelper.h"
#include "Engine/Engine.h"

void OGeometryEntityWidget::Draw()
{
	if (ImGui::CollapsingHeader(Geometry->Name.c_str()))
	{
		if (ImGui::BeginListBox("Submeshes"))
		{
			for (auto& Submesh : Geometry->DrawArgs)
			{
				auto name = Submesh.first.c_str();

				if (ImGui::Selectable(name, SelectedSubmesh == name))
				{
					SelectedSubmesh = name;
				}
			}
			ImGui::EndListBox();
		}

		if (SelectedSubmesh != "" && Geometry->DrawArgs.contains(SelectedSubmesh))
		{
			auto& Submesh = Geometry->DrawArgs[SelectedSubmesh];
			ImGui::SeparatorText("Submesh parameters");
			ImGui::Text("Submesh %s", SelectedSubmesh.c_str());
			if (ImGui::BeginListBox("Vertices"))
			{
				for (auto& Vertex : *Submesh.Vertices)
				{
					ImGui::SliderFloat3("Vertex", &Vertex.x, -1000.0f, 1000.0f);
				}
				ImGui::EndListBox();
			}

			if (ImGui::BeginListBox("Indices"))
			{
				for (auto& index : *Submesh.Indices)
				{
					int32_t idx = 0;
					ImGui::SliderInt("Index", &idx, std::numeric_limits<int16_t>().min(), std::numeric_limits<int16_t>().max());
					index = static_cast<int16_t>(idx);
				}
			}
		}

		if (ImGui::Button("Rebuild geometry"))
		{
			bRebuildRequested = true;
		}
	}
}

void OGeometryEntityWidget::Update()
{
	IWidget::Update();

	if (bRebuildRequested)
	{
		Engine->RebuildGeometry(Geometry->Name);
	}
}