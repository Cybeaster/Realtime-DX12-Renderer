#include "SsaoNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Window/Window.h"
ORenderTargetBase* OSSAONode::Execute(ORenderTargetBase* RenderTarget)
{
	DrawNormals();
	DrawSSAO();
	BlurSSAO();
	OEngine::Get()->GetWindow()->SetViewport(CommandQueue->GetCommandList().Get());
	return RenderTarget;
}

void OSSAONode::DrawNormals()
{
	auto frameResource = OEngine::Get()->CurrentFrameResource;
	auto ssao = OEngine::Get()->GetSSAORT();
	OEngine::Get()->GetWindow()->SetViewport(CommandQueue->GetCommandList().Get());
	CommandQueue->ResourceBarrier(ssao->GetNormalMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandQueue->SetRenderTarget(ssao, OSSAORenderTarget::ESubtargets::NormalSubtarget);
	auto pso = FindPSOInfo(SPSOType::DrawNormals);

	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource("cbBuffer", frameResource->PassCB->GetGPUAddress(), pso);
	OEngine::Get()->DrawRenderItems(pso, SRenderLayer::Opaque);
	CommandQueue->ResourceBarrier(ssao->GetNormalMap(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::DrawSSAO()
{
	auto frameResource = OEngine::Get()->CurrentFrameResource;
	auto ssao = OEngine::Get()->GetSSAORT();
	auto pso = FindPSOInfo(SPSOType::SSAO);
	CommandQueue->ResourceBarrier(ssao->GetAmbientMap0(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandQueue->SetRenderTarget(ssao, OSSAORenderTarget::ESubtargets::AmbientSubtarget0);
	CommandQueue->SetResource("cbSsao", frameResource->SsaoCB->GetGPUAddress(), pso);
	CommandQueue->SetResource("gNormalMap", ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gRandomVectorMap", ssao->GetRandomVectorMapSRV().GPUHandle, pso);
	OEngine::Get()->DrawFullScreenQuad();
	CommandQueue->ResourceBarrier(ssao->GetAmbientMap0(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::BlurSSAO()
{
	SetPSO(SPSOType::SSAOBlur);
	auto ssao = OEngine::Get()->GetSSAORT();
	auto frameRes = OEngine::Get()->CurrentFrameResource;
	for (int i = 0; i < 3; i++)
	{
		auto srv = ssao->GetAmbientMap0SRV();
		auto rtv = ssao->GetAmbientMap1RTV();
		ssao->SetHorizontalBlur(true);
		BlurSSAO(OSSAORenderTarget::ESubtargets::AmbientSubtarget1, &srv, &rtv);
		srv = ssao->GetAmbientMap1SRV();
		rtv = ssao->GetAmbientMap0RTV();
		ssao->SetHorizontalBlur(false);
		BlurSSAO(OSSAORenderTarget::ESubtargets::AmbientSubtarget0, &srv, &rtv);
	}
}

void OSSAONode::BlurSSAO(OSSAORenderTarget::ESubtargets OutputTarget, const SDescriptorPair* InputSRV, const SDescriptorPair* OutputRTV)
{
	auto ssao = OEngine::Get()->GetSSAORT();
	auto output = ssao->GetSubresource(OutputTarget);
	CommandQueue->ResourceBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
	float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CommandQueue->GetCommandList()->ClearRenderTargetView(OutputRTV->CPUHandle, clearValue, 0, nullptr); //TODO Make support without calling command list directly
	CommandQueue->GetCommandList()->OMSetRenderTargets(1, &OutputRTV->CPUHandle, true, nullptr);

	auto pso = FindPSOInfo(SPSOType::SSAOBlur);
	CommandQueue->SetResource("gNormalMap", ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gInputMap", InputSRV->GPUHandle, pso);
	CommandQueue->SetResource("cbRootConstants", ssao->GetBlurCBAddress(), pso);
	OEngine::Get()->DrawFullScreenQuad();
	CommandQueue->ResourceBarrier(output, D3D12_RESOURCE_STATE_GENERIC_READ);
}
