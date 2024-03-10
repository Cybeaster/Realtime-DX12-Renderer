
#include "PostProcessNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"
#include "Window/Window.h"
ORenderTargetBase* OPostProcessNode::Execute(ORenderTargetBase* RenderTarget)
{
	CommandQueue->CopyResourceTo(Window, RenderTarget);
	CommandQueue->ResourceBarrier(RenderTarget, D3D12_RESOURCE_STATE_GENERIC_READ);
	RenderTarget = CommandQueue->SetRenderTarget(Window);

	//DrawSobel(RenderTarget);
	//DrawBlurFilter(RenderTarget);
	return RenderTarget;
}
void OPostProcessNode::SetupCommonResources()
{
	Window = OEngine::Get()->GetWindow();
}

void OPostProcessNode::DrawSobel(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	SetPSO(SPSOType::SobelFilter);
	auto [executed, result] = engine->GetSobelFilter()->Execute(RenderTarget->GetSRV().GPUHandle);
	if (executed)
	{
		engine->DrawCompositeShader(result);
	}
}

void OPostProcessNode::DrawBlurFilter(ORenderTargetBase* RenderTarget)
{
	auto engine = OEngine::Get();
	engine->GetBlurFilter()->Execute(
	    FindPSOInfo(SPSOType::HorizontalBlur),
	    FindPSOInfo(SPSOType::VerticalBlur),
	    RenderTarget->GetResource());
	engine->GetBlurFilter()->OutputTo(RenderTarget->GetResource());

	SetPSO(SPSOType::BilateralBlur);
	engine->GetBilateralBlurFilter()->Execute(FindPSOInfo(SPSOType::BilateralBlur), RenderTarget->GetResource());
	engine->GetBilateralBlurFilter()->OutputTo(RenderTarget->GetResource());
}

void OPostProcessNode::DrawComposite(D3D12_GPU_DESCRIPTOR_HANDLE Input, D3D12_GPU_DESCRIPTOR_HANDLE Input2)
{
	const auto commandList = CommandQueue->GetCommandList();
	SetPSO(SPSOType::Composite);
	commandList->SetGraphicsRootDescriptorTable(0, Input2);
	commandList->SetGraphicsRootDescriptorTable(1, Input);
	OEngine::Get()->DrawFullScreenQuad();
}