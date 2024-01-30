#pragma once
#include "DXHelper.h"

#include <imgui.h>

class OUIManager
{
public:
	void InitContext(ID3D12Device2* Device, HWND Hwnd, UINT NumFramesInLight, ID3D12DescriptorHeap* SRVDescriptorHeap);
	void PreRender();
	void Render();
	void PostRender(ID3D12GraphicsCommandList* List);
	void Destroy();

private:
	ImGuiIO* IO = nullptr;
};
