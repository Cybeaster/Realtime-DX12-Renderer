
#include "RenderGraphWidget.h"

#include "RenderGraph/Graph/RenderGraph.h"
ORenderGraphWidget::ORenderGraphWidget(ORenderGraph* InGraph)
    : Graph(InGraph)
{
}

void ORenderGraphWidget::Draw()
{
	if (ImGui::CollapsingHeader("Render Graph"))
	{
		if (ImGui::TreeNode("Nodes"))
		{
			auto curr = Graph->GetHead();
			while (curr)
			{
				if (ImGui::Selectable(curr->GetNodeInfo().Name.c_str(), Node == curr))
				{
					Node = curr;
				}
				curr = Graph->GetNext(curr);
			}
			ImGui::TreePop();
		}
		if (Node)
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
	}
}