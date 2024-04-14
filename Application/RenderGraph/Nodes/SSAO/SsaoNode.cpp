#include "SsaoNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Profiler.h"
#include "Window/Window.h"
ORenderTargetBase* OSSAONode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	DrawNormals();
	DrawSSAO();
	BlurSSAO();
	CommandQueue->SetRenderTarget(RenderTarget);
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
	auto resource = OEngine::Get()->CurrentFrameResource;

	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart(), pso);
	OEngine::Get()->DrawRenderItems(pso, SRenderLayers::Opaque);
	CommandQueue->ResourceBarrier(ssao->GetNormalMap(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::DrawSSAO()
{
	PROFILE_SCOPE();
	auto frameResource = OEngine::Get()->CurrentFrameResource;
	auto ssao = OEngine::Get()->GetSSAORT();
	auto pso = FindPSOInfo(SPSOType::SSAO);
	CommandQueue->ResourceBarrier(ssao->GetAmbientMap0(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandQueue->CopyResourceTo(ssao->GetDepthMap(), OEngine::Get()->GetWindow()->GetCurrentDepthStencilBuffer());
	CommandQueue->SetRenderTarget(ssao, OSSAORenderTarget::ESubtargets::AmbientSubtarget0);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource("cbSsao", frameResource->SsaoCB->GetGPUAddress(), pso);
	CommandQueue->SetResource("gNormalMap", ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gRandomVecMap", ssao->GetRandomVectorMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gDepthMap", ssao->GetDepthMapSRV().GPUHandle, pso);
	OEngine::Get()->DrawFullScreenQuad();
	CommandQueue->ResourceBarrier(ssao->GetAmbientMap0(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::BlurSSAO()
{
	PROFILE_SCOPE();
	SetPSO(SPSOType::SSAOBlur);
	auto frameResource = OEngine::Get()->CurrentFrameResource;
	auto ssao = OEngine::Get()->GetSSAORT();
	auto pso = FindPSOInfo(SPSOType::SSAOBlur);
	CommandQueue->SetResource("gNormalMap", ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("gDepthMap", ssao->GetDepthMapSRV().GPUHandle, pso);
	CommandQueue->SetResource("cbSsao", frameResource->SsaoCB->GetGPUAddress(), pso);
	auto cmdList = CommandQueue->GetCommandList();
	auto srv0 = ssao->GetAmbientMap0SRV();
	auto rtv0 = ssao->GetAmbientMap0RTV();
	auto srv1 = ssao->GetAmbientMap1SRV();
	auto rtv1 = ssao->GetAmbientMap1RTV();

	for (int i = 0; i < 5; i++)
	{
		CommandQueue->SetResource("cbRootConstants", ssao->GetHBlurCBAddress(), pso);
		BlurSSAO(OSSAORenderTarget::ESubtargets::AmbientSubtarget1, &srv0, &rtv1);
		CommandQueue->SetResource("cbRootConstants", ssao->GetVBlurCBAddress(), pso);
		BlurSSAO(OSSAORenderTarget::ESubtargets::AmbientSubtarget0, &srv1, &rtv0);
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
	CommandQueue->SetResource("gInputMap", InputSRV->GPUHandle, pso);
	const auto commandList = CommandQueue->GetCommandList();
	OEngine::Get()->DrawFullScreenQuad();
	CommandQueue->ResourceBarrier(output, D3D12_RESOURCE_STATE_GENERIC_READ);
}
