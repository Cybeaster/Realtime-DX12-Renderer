#include "UiManager.h"

#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "imgui.h"
void OUIManager::InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	IO = &ImGui::GetIO();
	IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(Hwnd);
	ImGui_ImplDX12_Init(
	    Device,
	    NumFramesInLight,
	    DXGI_FORMAT_R8G8B8A8_UNORM,
	    SRVDescriptorHeap,
	    SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	    SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	ImGui::StyleColorsDark();
}

void OUIManager::PreRender()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void OUIManager::Render()
{
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