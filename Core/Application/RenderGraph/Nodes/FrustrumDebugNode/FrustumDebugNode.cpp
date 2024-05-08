//
// Created by Cybea on 29/04/2024.
//

#include "FrustumDebugNode.h"

#include "Engine/Engine.h"
#include "Window/Window.h"
ORenderTargetBase* OFrustumDebugNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	auto camera = OEngine::Get()->GetWindow()->GetCamera();
	auto pso = FindPSOInfo(PSO);
	SetPSO(PSO);
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), resource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CAMERA_MATRIX), resource->CameraMatrixBuffer->GetGPUAddress(), pso);
	OEngine::Get()->DrawFullScreenQuad();

	auto shadowMaps = OEngine::Get()->GetShadowMaps();
	for (auto shadow : shadowMaps)
	{
		CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), shadow->GetPassConstantAddresss(), pso);
		OEngine::Get()->DrawFullScreenQuad();
	}
	return RenderTarget;
}