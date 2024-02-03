#include "WindowTest.h"

void OWindowTest::OnRender(const UpdateEventArgs& Arg)
{
	auto window = Window.lock();
	auto engine = Engine.lock();

	auto list = engine->GetCommandQueue()->GetCommandList();

	window->TransitionResource(list, window->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	list->RSSetViewports(1, &window->Viewport);
	list->RSSetScissorRects(1, &window->ScissorRect);

	FLOAT color[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	auto rtv = window->CurrentBackBufferView();
	auto dsv = window->GetDepthStensilView();

	list->ClearRenderTargetView(rtv, color, 0, nullptr);
	list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto descHeapStart = window->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	list->OMSetRenderTargets(1, &rtv, true, &descHeapStart);

	window->TransitionResource(list, window->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	engine->GetCommandQueue()->ExecuteCommandList();
	window->Present();
	engine->FlushGPU();
}