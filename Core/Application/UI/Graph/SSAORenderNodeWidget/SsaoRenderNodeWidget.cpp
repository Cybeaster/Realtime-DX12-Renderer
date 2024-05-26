

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
	const auto rt = engine->GetSSAORT().lock();
	ImGui::SeparatorText("SSAO Parameters");
	ImGui::DragFloat("OcclusionRadius", &rt->OcclusionRadius, 0.01f, 0.0f, OSSAORenderTarget::MaxBlurRadius);
	ImGui::DragFloat("OcclusionFadeStart", &rt->OcclusionFadeStart, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("OcclusionFadeEnd", &rt->OcclusionFadeEnd, 0.01f, 0.0f, 10.0f);

	auto ssao = Cast<OSSAONode>(Node)->GetSSAORT().lock();
	ImGui::SeparatorText("SSAO Ambient Map");
	ImGui::Image(PtrCast(ssao->GetAmbientMap0SRV().GPUHandle.ptr), ImVec2(200, 200));
	ImGui::SeparatorText("SSAO Normal Map");
	ImGui::Image(PtrCast(ssao->GetNormalMapSRV().GPUHandle.ptr), ImVec2(200, 200));
	ImGui::SeparatorText("SSAO Depth Map");
	ImGui::Image(PtrCast(ssao->GetDepthMapSRV().GPUHandle.ptr), ImVec2(200, 200));
}

OSSAONode* OSSAORenderNodeWidget::GetRenderNode() const
{
	return Cast<OSSAONode>(Node);
}