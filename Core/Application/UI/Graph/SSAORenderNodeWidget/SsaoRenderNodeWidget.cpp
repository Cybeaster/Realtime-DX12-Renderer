

#include "SsaoRenderNodeWidget.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "RenderGraph/Nodes/RenderNode.h"
#include "RenderGraph/Nodes/SSAO/SsaoNode.h"
#include "Statics.h"

void OSSAORenderNodeWidget::Draw()
{
	ORenderNodeWidgetBase::Draw();
	auto engine = OEngine::Get();
	engine->GetMainPassCB().SSAOEnabled = Node->GetNodeInfo().bEnable;
	auto rt = engine->GetSSAORT();
	ImGui::SeparatorText("SSAO Parameters");
	ImGui::DragFloat("OcclusionRadius", &rt->OcclusionRadius, 0.01f, 0.0f, OSSAORenderTarget::MaxBlurRadius);
	ImGui::DragFloat("OcclusionFadeStart", &rt->OcclusionFadeStart, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("OcclusionFadeEnd", &rt->OcclusionFadeEnd, 0.01f, 0.0f, 10.0f);
}

OSSAONode* OSSAORenderNodeWidget::GetRenderNode() const
{
	return Cast<OSSAONode>(Node);
}