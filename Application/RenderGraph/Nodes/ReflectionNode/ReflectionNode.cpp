
#include "ReflectionNode.h"
#include "Engine/Engine.h"
ORenderTargetBase* OReflectionNode::Execute(ORenderTargetBase* RenderTarget)
{
	/*auto cube = OEngine::Get()->GetCubeRenderTarget();
	auto cmdList = CommandQueue->GetCommandList();
	cube->SetViewport(CommandQueue);
	Utils::ResourceBarrier(cmdList.Get(), cube->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	UINT passCBByteSize = Utils::CalcBufferByteSize(sizeof(SPassConstants));
	auto resource = OEngine::Get()->CurrentFrameResources->PassCB->GetResource()->GetGPUVirtualAddress();
	for (size_t i = 0; i < cube->GetNumRTVRequired(); i++)
	{
		auto& rtv = cube->GetRTVHandle()[i];
		auto& dsv = cube->GetDSVHandle();
		CHECK(rtv.CPUHandle.ptr != 0);
		CHECK(dsv.CPUHandle.ptr != 0);

		cmdList->ClearRenderTargetView(rtv.CPUHandle, DirectX::Colors::LightSteelBlue, 0, nullptr);
		cmdList->ClearDepthStencilView(dsv.CPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv.CPUHandle, true, &dsv.CPUHandle);

		cmdList->SetGraphicsRootConstantBufferView(2, resource + (i + 1) * passCBByteSize);

		OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::Opaque);
		OEngine::Get()->DrawRenderItems(SPSOType::WavesRender, SRenderLayer::Waves);
		OEngine::Get()->DrawRenderItems(SPSOType::Sky, SRenderLayer::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cube->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	PSO->RootSignature->SetDescriptorTable("gCubeMap", cube->GetSRVHandle().GPUHandle, cmdList.Get());
	OEngine::Get()->DrawRenderItems(SPSOType::Opaque, SRenderLayer::OpaqueDynamicReflections);
	PSO->RootSignature->SetDescriptorTable("gCubeMap", GetSkyTextureSRV(), cmdList.Get());*/
	return RenderTarget;
}