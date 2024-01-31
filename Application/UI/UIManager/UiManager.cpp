#include "UiManager.h"

#include "Logger.h"
#include "RenderConstants.h"

void OUIManager::InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap, SRenderObjectDescriptor& OutDescriptor)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	IO = &ImGui::GetIO();
	IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(Hwnd);
	ImGui_ImplDX12_Init(
	    Device,
	    NumFramesInLight,
	    SRenderConstants::BackBufferFormat,
	    SRVDescriptorHeap,
	    OutDescriptor.CPUSRVescriptor,
	    OutDescriptor.GPUSRVDescriptor);

	ImGui::StyleColorsDark();
	UpdateDescriptors(OutDescriptor);
}

void OUIManager::Draw()
{
	PostInputUpdate();

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Main Menu");
	for (const auto& widget : Widgets)
	{
		widget->Update();
		widget->Draw();
	}
	ImGui::End();

	ImGui::ShowDemoWindow();

	ImGui::Render();
}

void OUIManager::PostRender(ID3D12GraphicsCommandList* List)
{
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), List);
}

void OUIManager::Destroy()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();
}

void OUIManager::OnMouseButtonPressed(MouseButtonEventArgs& Args)
{
	switch (Args.Button)
	{
	case MouseButtonEventArgs::None:
		break;
	case MouseButtonEventArgs::Left:
		IO->MouseDown[0] = true;
		break;
	case MouseButtonEventArgs::Right:
		IO->MouseDown[1] = true;
		break;
	case MouseButtonEventArgs::Middel:
		IO->MouseDown[2] = true;
		break;
	}
}

void OUIManager::OnMouseButtonReleased(MouseButtonEventArgs& Args)
{
	switch (Args.Button)
	{
	case MouseButtonEventArgs::None:
		break;
	case MouseButtonEventArgs::Left:
		IO->MouseDown[0] = false;
		break;
	case MouseButtonEventArgs::Right:
		IO->MouseDown[1] = false;
		break;
	case MouseButtonEventArgs::Middel:
		IO->MouseDown[2] = false;
		break;
	}
}

void OUIManager::OnResize(ResizeEventArgs& Args)
{
	if (IO)
	{
		IO->DisplaySize = ImVec2(static_cast<float>(Args.Width), static_cast<float>(Args.Height));
	}
}

void OUIManager::UpdateDescriptors(SRenderObjectDescriptor& OutDescriptor)
{
	OutDescriptor.OffsetSRV(GetNumDescriptors());
}

void OUIManager::PostInputUpdate()
{
	bIsInFocus = ImGui::IsAnyItemFocused() || ImGui::IsAnyItemActive() || ImGui::IsItemClicked();
	LOG(Widget, Log, "IsInFocus: {}", bIsInFocus);
}
bool OUIManager::IsInFocus()
{
	return bIsInFocus;
}
