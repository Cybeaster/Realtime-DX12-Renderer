//
// Created by Cybea on 24/02/2024.
//

#include "PickerTableWidget.h"

#include "imgui_internal.h"
void OPickerTableWidget::Draw()
{
	if (ImGui::CollapsingHeader(HeadderName.c_str()))
	{
		// Calculate the width for the two child windows
		float widthAvailable = ImGui::GetContentRegionAvail().x - SplitterWidth;
		float width1 = widthAvailable * SplitterPercent;
		float width2 = widthAvailable - width1;

		// First child window (left side with material list)
		ImGui::BeginChild(ListName.c_str(), ImVec2(width1, 0), true);
		DrawTable();
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
		ImGui::BeginChild(PropertyName.c_str(), ImVec2(width2, 0), true);
		DrawProperty();
		ImGui::EndChild();
	}
}