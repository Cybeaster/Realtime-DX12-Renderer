//
// Created by Cybea on 06/03/2024.
//

#include "PostProcessNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"
ORenderTargetBase* OPostProcessNode::Execute(ORenderTargetBase* RenderTarget)
{
	CommandQueue->ResourceBarrier(RenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandQueue->ResourceBarrier(Window, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	auto rtv = Window->CurrentBackBufferView();
	auto dsv = Window->GetDepthStensilView();
	CommandQueue->GetCommandList()->OMSetRenderTargets(1, &rtv, true, &dsv);

	DrawSobel(RenderTarget);
	DrawBlurFilter(RenderTarget);
	return RenderTarget;
}
void OPostProcessNode::SetupCommonResources()
{
	Window = OEngine::Get()->GetWindow();
}

void OPostProcessNode::DrawSobel(ORenderTargetBase*& RenderTarget)
{
	auto engine = OEngine::Get();
	SetPSO(SPSOType::SobelFilter);
	auto [executed, result] = engine->GetSobelFilter()->Execute(RenderTarget->GetSRV().GPUHandle);
	if (executed)
	{
		engine->DrawCompositeShader(result);
	}
	else
	{
		CommandQueue->CopyResourceTo(Window, RenderTarget);
	}
	RenderTarget = Window;
}

void OPostProcessNode::DrawBlurFilter(const ORenderTargetBase* RenderTarget)
{
	auto horizontal = FindPSOInfo(SPSOType::HorizontalBlur);
	auto vertical = FindPSOInfo(SPSOType::VerticalBlur);
	auto engine = OEngine::Get();
	engine->GetBlurFilter()->Execute(horizontal->RootSignature->RootSignatureParams.RootSignature.Get(),
	                                 horizontal->PSO.Get(),
	                                 vertical->PSO.Get(),
	                                 RenderTarget->GetResource());
	engine->GetBlurFilter()->OutputTo(RenderTarget->GetResource());

	SetPSO(SPSOType::BilateralBlur);
	engine->GetBilateralBlurFilter()->Execute(RenderTarget->GetResource());

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