
#include "PostProcessNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"
#include "Profiler.h"
#include "Window/Window.h"
ORenderTargetBase* OPostProcessNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	DrawBlurFilter(RenderTarget);
	DrawSobel(RenderTarget);
	return RenderTarget;
}
void OPostProcessNode::SetupCommonResources()
{
	Window = OEngine::Get()->GetWindow();
}

void OPostProcessNode::DrawSobel(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	auto sobel = engine->GetSobelFilter();
	if (sobel->IsEnabled())
	{
		if (sobel->Execute(FindPSOInfo(SPSOTypes::SobelFilter), RenderTarget))
		{
			if (sobel->IsPureSobel())
			{
				CommandQueue->CopyResourceTo(RenderTarget->GetResource(), engine->GetSobelFilter()->GetOutput());
			}
			else
			{
				DrawComposite(engine->GetSobelFilter()->GetInputSRV().GPUHandle, engine->GetSobelFilter()->GetOutputSRV().GPUHandle);
			}
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
	OEngine::Get()->DrawFullScreenQuad(pso);
}