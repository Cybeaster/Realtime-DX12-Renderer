//
// Created by Cybea on 07/04/2024.
//

#include "ORenderNodeWidgetBase.h"

#include "RenderGraph/Nodes/RenderNode.h"

void ORenderNodeWidgetBase::Draw()
{
	ImGui::SeparatorText("Node Info");
	ImGui::Text("Name: %s", Node->GetNodeInfo().Name.c_str());
	ImGui::Text("Layer: %s", Node->GetNodeInfo().RenderLayer.c_str());
	ImGui::Text("PSO: %s", Node->GetNodeInfo().PSOType.c_str());
	ImGui::Text("Enable: %s", Node->GetNodeInfo().bEnable ? "True" : "False");
	bool bEnabled = Node->GetNodeInfo().bEnable;
	if (ImGui::Checkbox("Enable", &bEnabled))
	{
		Node->SetNodeEnabled(bEnabled);
	}
}