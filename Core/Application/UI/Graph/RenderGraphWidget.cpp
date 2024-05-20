
#include "RenderGraphWidget.h"

#include "MainPassNode/MainPassNodeWidget.h"
#include "RenderGraph/Graph/RenderGraph.h"
#include "SSAORenderNodeWidget/SsaoRenderNodeWidget.h"
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
			if (Node->GetNodeInfo().Name == "SSAO")
			{
				SSAONodeWidget->SetNode(Node);
				SSAONodeWidget->Draw();
			}
			else if (Node->GetNodeInfo().Name == "Opaque")
			{
				OpaquePassNodeWidget->SetNode(Node);
				OpaquePassNodeWidget->Draw();
			}
			else
			{
				BaseNodeWidget->SetNode(Node);
				BaseNodeWidget->Draw();
			}
		}
	}
}

void ORenderGraphWidget::InitWidget()
{
	SSAONodeWidget = MakeWidget<OSSAORenderNodeWidget>();
	BaseNodeWidget = MakeWidget<ORenderNodeWidgetBase>();
	OpaquePassNodeWidget = MakeWidget<OOpaquePassNodeWidget>();
}