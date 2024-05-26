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
	OEngine::Get()->GetWindow().lock()->SetViewport(CommandQueue->GetCommandList().Get());
	return RenderTarget;
}

weak_ptr<OSSAORenderTarget> OSSAONode::GetSSAORT() const
{
	return OEngine::Get()->GetSSAORT();
}

void OSSAONode::DrawNormals()
{
	auto ssao = OEngine::Get()->GetSSAORT().lock();
	CommandQueue->ResourceBarrier(ssao->GetNormalMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandQueue->SetRenderTarget(ssao.get(), OSSAORenderTarget::ESubtargets::NormalSubtarget);
	auto pso = FindPSOInfo(SPSOTypes::DrawNormals);
	auto resource = OEngine::Get()->CurrentFrameResource;

	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), resource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(MATERIAL_DATA), resource->MaterialBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(TEXTURE_MAPS), OEngine::Get()->TexturesStartAddress.GPUHandle, pso);
	OEngine::Get()->DrawRenderItems(pso, SRenderLayers::Opaque);
	CommandQueue->ResourceBarrier(ssao->GetNormalMap(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::DrawSSAO()
{
	PROFILE_SCOPE();
	auto frameResource = OEngine::Get()->CurrentFrameResource;
	auto ssao = OEngine::Get()->GetSSAORT().lock();
	auto pso = FindPSOInfo(SPSOTypes::SSAO);
	CommandQueue->ResourceBarrier(ssao->GetDepthMap(), D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandQueue->SetRenderTarget(ssao.get(), OSSAORenderTarget::ESubtargets::AmbientSubtarget0);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_SSAO), frameResource->SsaoCB->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(NORMAL_MAP), ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(RANDOM_VEC_MAP), ssao->GetRandomVectorMapSRV().GPUHandle, pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(DEPTH_MAP), ssao->GetDepthMapSRV().GPUHandle, pso);
	OEngine::Get()->DrawFullScreenQuad(pso);
	CommandQueue->ResourceBarrier(ssao->GetAmbientMap0(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::BlurSSAO()
{
	PROFILE_SCOPE();
	SetPSO(SPSOTypes::SSAOBlur);
	auto frameResource = OEngine::Get()->CurrentFrameResource;
	auto ssao = OEngine::Get()->GetSSAORT().lock();
	auto pso = FindPSOInfo(SPSOTypes::SSAOBlur);
	CommandQueue->SetResource(STRINGIFY_MACRO(NORMAL_MAP), ssao->GetNormalMapSRV().GPUHandle, pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(DEPTH_MAP), ssao->GetDepthMapSRV().GPUHandle, pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_SSAO), frameResource->SsaoCB->GetGPUAddress(), pso);
	auto cmdList = CommandQueue->GetCommandList();
	auto srv0 = ssao->GetAmbientMap0SRV();
	auto rtv0 = ssao->GetAmbientMap0RTV();
	auto srv1 = ssao->GetAmbientMap1SRV();
	auto rtv1 = ssao->GetAmbientMap1RTV();

	for (int i = 0; i < 5; i++)
	{
		CommandQueue->SetResource(STRINGIFY_MACRO(CB_ROOT_CONSTANTS), ssao->GetHBlurCBAddress(), pso);
		BlurSSAO(OSSAORenderTarget::ESubtargets::AmbientSubtarget1, &srv0, &rtv1);
		CommandQueue->SetResource(STRINGIFY_MACRO(CB_ROOT_CONSTANTS), ssao->GetVBlurCBAddress(), pso);
		BlurSSAO(OSSAORenderTarget::ESubtargets::AmbientSubtarget0, &srv1, &rtv0);
	}
	CommandQueue->ResourceBarrier(ssao->GetDepthMap(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void OSSAONode::BlurSSAO(OSSAORenderTarget::ESubtargets OutputTarget, const SDescriptorPair* InputSRV, const SDescriptorPair* OutputRTV)
{
	auto ssao = OEngine::Get()->GetSSAORT().lock();
	auto output = ssao->GetSubresource(OutputTarget);
	CommandQueue->ResourceBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
	float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CommandQueue->GetCommandList()->ClearRenderTargetView(OutputRTV->CPUHandle, clearValue, 0, nullptr); //TODO Make support without calling command list directly
	CommandQueue->GetCommandList()->OMSetRenderTargets(1, &OutputRTV->CPUHandle, true, nullptr);
	auto pso = FindPSOInfo(SPSOTypes::SSAOBlur);
	CommandQueue->SetResource("gInputMap", InputSRV->GPUHandle, pso);
	const auto commandList = CommandQueue->GetCommandList();
	OEngine::Get()->DrawFullScreenQuad(pso);
	CommandQueue->ResourceBarrier(output, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void OSSAONode::Update()
{
	ORenderNode::Update();
	const auto ssao = OEngine::Get()->GetSSAORT().lock();
	ssao->SetEnabled(GetNodeInfo().bEnable);
}
