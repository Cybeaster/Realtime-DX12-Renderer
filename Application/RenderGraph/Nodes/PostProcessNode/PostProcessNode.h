
#pragma once
#include "RenderGraph/Nodes/RenderNode.h"

class OWindow;
class OPostProcessNode : public ORenderNode
{
public:
	virtual ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;
	void SetupCommonResources() override;

private:
	void DrawSobel(ORenderTargetBase*& RenderTarget);
	void DrawBlurFilter(ORenderTargetBase* RenderTarget);
	void DrawComposite(D3D12_GPU_DESCRIPTOR_HANDLE Input, D3D12_GPU_DESCRIPTOR_HANDLE Input2);
	OWindow* Window = nullptr;
};
