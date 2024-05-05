
#include "PostProcessNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"
#include "Profiler.h"
#include "Window/Window.h"
ORenderTargetBase* OPostProcessNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	DrawSobel(RenderTarget);
	DrawBlurFilter(Window);
	CommandQueue->ResourceBarrier(Window, D3D12_RESOURCE_STATE_RENDER_TARGET);
	return Window;
}
void OPostProcessNode::SetupCommonResources()
{
	Window = OEngine::Get()->GetWindow();
}

void OPostProcessNode::DrawSobel(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	if (engine->GetSobelFilter()->GetIsEnabled())
	{
		CommandQueue->ResourceBarrier(RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
		auto [executed, result] = engine->GetSobelFilter()->Execute(FindPSOInfo(SPSOTypes::SobelFilter), RenderTarget->GetSRV().GPUHandle);
		if (executed)
		{
			DrawComposite(RenderTarget->GetSRV().GPUHandle, result);
		}
	}
}

void OPostProcessNode::DrawBlurFilter(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	engine->GetBlurFilter()->Execute(
	    FindPSOInfo(SPSOTypes::HorizontalBlur),
	    FindPSOInfo(SPSOTypes::VerticalBlur),
	    RenderTarget->GetResource());
	engine->GetBlurFilter()->OutputTo(RenderTarget->GetResource());

	SetPSO(SPSOTypes::BilateralBlur);
	engine->GetBilateralBlurFilter()->Execute(FindPSOInfo(SPSOTypes::BilateralBlur), RenderTarget->GetResource());
	engine->GetBilateralBlurFilter()->OutputTo(RenderTarget->GetResource());
}

void OPostProcessNode::DrawComposite(D3D12_GPU_DESCRIPTOR_HANDLE Input, D3D12_GPU_DESCRIPTOR_HANDLE Input2)
{
	const auto commandList = CommandQueue->GetCommandList();
	auto pso = FindPSOInfo(SPSOTypes::Composite);
	SetPSO(SPSOTypes::Composite);
	CommandQueue->SetResource("gBaseMap", Input, pso);
	CommandQueue->SetResource("gEdgeMap", Input2, pso);
	OEngine::Get()->DrawFullScreenQuad();
}