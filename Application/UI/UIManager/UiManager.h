#pragma once
#include "../Widget.h"
#include "..\Filters\BilateralFilterWidget.h"
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"
#include "Events.h"
#include "Filters/BilateralBlur/BilateralBlurFilter.h"

class OUIManager : public OHierarchicalWidgetBase
    , public IRenderObject

{
public:
	void InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap, SRenderObjectDescriptor& OutDescriptor);
	void Draw() override;
	void PostRender(ID3D12GraphicsCommandList* List);
	void Destroy();

	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnKeyboardKeyPressed(KeyEventArgs& Args);
	void OnKeyboardKeyReleased(KeyEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);
	void OnResize(ResizeEventArgs& Args);

	void KeyMap();
	void UpdateDescriptors(SRenderObjectDescriptor& OutDescriptor) override;
	void BuildDescriptors(IDescriptor* Descriptor) override {}

	uint32_t GetNumDescriptors() const override
	{
		return 10;
	}

	void PostInputUpdate();

	bool IsInFocus() override;

private:
	bool bIsInFocus = false;
	ImGuiIO* IO = nullptr;

	float MangerWidth = 400;
	float MangerHeight = 800;
};
