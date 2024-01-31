#pragma once
#include "../Widget.h"
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"
#include "Events.h"
#include "Filters/BilateralBlur/BilateralBlurFilter.h"
#include "UI/Filters/FilterWidget.h"

class OUIManager : public IWidget
    , public IRenderObject

{
public:
	void InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap, SRenderObjectDescriptor& OutDescriptor);
	void Draw() override;
	void PostRender(ID3D12GraphicsCommandList* List);
	void Destroy();

	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnResize(ResizeEventArgs& Args);

	void UpdateDescriptors(SRenderObjectDescriptor& OutDescriptor) override;
	void BuildDescriptors(IDescriptor* Descriptor) override {}

	template<typename WidgetType, typename... Params>
	void MakeWidget(Params&&... Args)
	{
		Widgets.push_back(make_unique<WidgetType>(std::forward<Params>(Args))...);
	}

	uint32_t GetNumDescriptors() const override
	{
		return 10;
	}

	void PostInputUpdate();

	bool IsInFocus() override;

private:
	vector<unique_ptr<IWidget>> Widgets;
	bool bIsInFocus = false;
	ImGuiIO* IO = nullptr;
};
