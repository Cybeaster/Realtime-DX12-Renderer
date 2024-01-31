#pragma once
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"
#include "Events.h"

#include <imgui.h>

class OUIManager : public IRenderObject
{
public:
	void InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap, SRenderObjectDescriptor& OutDescriptor);
	void Render();
	void PostRender(ID3D12GraphicsCommandList* List);
	void Destroy();
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnResize(ResizeEventArgs& Args);
	uint32_t GetNumDescriptors() const override
	{
		return 10;
	}

	void UpdateDescriptors(SRenderObjectDescriptor& OutDescriptor) override;
	void BuildDescriptors(IDescriptor* Descriptor) override {}

private:
	bool IsEnabled = true;
	ImGuiIO* IO = nullptr;
};
