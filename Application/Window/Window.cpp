//
// Created by Cybea on 02/12/2023.
//

#include "Window.h"

#include "Application.h"
#include "Exception.h"
#include "Logger.h"

#include <DXHelper.h>
#include <Types.h>

using namespace Microsoft::WRL;
OWindow::OWindow(shared_ptr<OEngine> _Engine, HWND hWnd, const std::wstring& WindowName, int Width, int Height, bool vSync)
    : Hwnd(hWnd), Engine(_Engine), Name(WindowName), ClientWidth(Width), ClientHeight(Height), VSync(vSync), Fullscreen(false), FrameCounter(0)
{
	IsTearingSupported = _Engine->IsTearingSupported();

	SwapChain = CreateSwapChain();
	RTVDescriptorHeap = _Engine->CreateDescriptorHeap(BuffersCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	RTVDescriptorSize = _Engine->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	UpdateRenderTargetViews();
}
const wstring& OWindow::GetName() const
{
	return Name;
}

uint32_t OWindow::GetWidth() const
{
	return ClientWidth;
}

uint32_t OWindow::GetHeight() const
{
	return ClientHeight;
}

UINT OWindow::GetCurrentBackBufferIndex() const
{
	return CurrentBackBufferIndex;
}

UINT OWindow::Present()
{
	UINT syncInterval = VSync ? 1 : 0;
	UINT presentFlags = IsTearingSupported && !VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;

	ThrowIfFailed(SwapChain->Present(syncInterval, presentFlags));
	CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	return CurrentBackBufferIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE OWindow::GetCurrentRenderTargetView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	                                     CurrentBackBufferIndex,
	                                     RTVDescriptorSize);
}

Microsoft::WRL::ComPtr<ID3D12Resource> OWindow::GetCurrentBackBuffer() const
{
	return BackBuffers[CurrentBackBufferIndex];
}

bool OWindow::IsVSync() const
{
	return VSync;
}

void OWindow::SetVSync(bool vSync)
{
	VSync = vSync;
}

void OWindow::ToggleVSync()
{
	SetVSync(!VSync);
}

bool OWindow::IsFullScreen() const
{
	return Fullscreen;
}

void OWindow::SetFullscreen(bool _Fullscreen)
{
	if (Fullscreen != _Fullscreen)
	{
		Fullscreen = _Fullscreen;
		if (Fullscreen)
		{
			// Store the current window dimensions so they can be restored
			// when switching out of fullscreen state.
			::GetWindowRect(Hwnd, &WindowRect);

			// Set the window style to a borderless window so the client area fills
			// the entire screen.
			constexpr UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

			::SetWindowLongW(Hwnd, GWL_STYLE, windowStyle);

			// Query the name of the nearest display device for the window.
			// This is required to set the fullscreen dimensions of the window
			// when using a multi-monitor setup.
			const HMONITOR hMonitor = ::MonitorFromWindow(Hwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorInfo);

			::SetWindowPos(Hwnd, HWND_TOPMOST, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(Hwnd, SW_MAXIMIZE);
		}
		else
		{
			// Restore all the window decorators.
			::SetWindowLong(Hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(Hwnd, HWND_NOTOPMOST, WindowRect.left, WindowRect.top, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(Hwnd, SW_NORMAL);
		}
	}
}

void OWindow::ToggleFullscreen()
{
	SetFullscreen(!Fullscreen);
}

void OWindow::Show()
{
	::ShowWindow(Hwnd, SW_SHOW);
}

void OWindow::Hide()
{
	::ShowWindow(Hwnd, SW_HIDE);
}

void OWindow::RegsterWindow(shared_ptr<OEngine> Other)
{
	Engine = Other;
}

void OWindow::Destroy()
{
	if (const auto engine = Engine.lock())
	{
		engine->OnWindowDestroyed();
	}

	if (Hwnd && !::DestroyWindow(Hwnd))
	{
		LOG(Error, "Failed to destroy window.");
	}
}

void OWindow::OnUpdate(UpdateEventArgs& Event)
{
	UpdateClock.Tick();
	if (auto engine = Engine.lock())
	{
		FrameCounter++;
		UpdateEventArgs args(UpdateClock.GetDeltaSeconds(), UpdateClock.GetTotalSeconds());
		engine->OnUpdate(args);
	}
}

void OWindow::OnRender(RenderEventArgs& Event)
{
	RenderClock.Tick();

	if (const auto engine = Engine.lock())
	{
		RenderEventArgs renderEventArgs(RenderClock.GetDeltaSeconds(), RenderClock.GetTotalSeconds());
		engine->OnRender(renderEventArgs);
	}
}

void OWindow::OnKeyPressed(KeyEventArgs& Event)
{
	if (const auto engine = Engine.lock())
	{
		engine->OnKeyPressed(Event);
	}
}

void OWindow::OnKeyReleased(KeyEventArgs& Event)
{
	if (const auto engine = Engine.lock())
	{
		engine->OnKeyPressed(Event);
	}
}

void OWindow::OnMouseMoved(MouseMotionEventArgs& Event)
{
	if (const auto engine = Engine.lock())
	{
		engine->OnMouseMoved(Event);
	}
}

void OWindow::OnMouseButtonPressed(MouseButtonEventArgs& Event)
{
	if (const auto engine = Engine.lock())
	{
		engine->OnMouseButtonPressed(Event);
	}
}

void OWindow::OnMouseButtonReleased(MouseButtonEventArgs& Event)
{
	if (const auto engine = Engine.lock())
	{
		engine->OnMouseButtonReleased(Event);
	}
}

void OWindow::OnMouseWheel(MouseWheelEventArgs& Event)
{
	if (const auto engine = Engine.lock())
	{
		engine->OnMouseWheel(Event);
	}
}

void OWindow::OnResize(ResizeEventArgs& Event)
{
	if (ClientWidth != Event.Width || ClientHeight != Event.Height)
	{
		ClientWidth = std::max(1, Event.Width);
		ClientHeight = std::max(1, Event.Height);

		Engine.lock()->FlushGPU();

		for (int i = 0; i < BuffersCount; ++i)
		{
			BackBuffers[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(SwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(SwapChain->ResizeBuffers(BuffersCount, ClientWidth, ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();

		UpdateRenderTargetViews();
	}

	if (const auto engine = Engine.lock())
	{
		engine->OnResize(Event);
	}
}

Microsoft::WRL::ComPtr<IDXGISwapChain4> OWindow::CreateSwapChain()
{
	CHECK(Hwnd != nullptr, "HWND is null.");
	const auto engine = Engine.lock();
	ComPtr<IDXGISwapChain4> swapChain4;
	ComPtr<IDXGIFactory4> factory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = ClientWidth;
	swapChainDesc.Height = ClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BuffersCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = IsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	const auto commandQueue = engine->GetCommandQueue()->GetCommandQueue();

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(factory4->CreateSwapChainForHwnd(
	    commandQueue.Get(),
	    Hwnd,
	    &swapChainDesc,
	    nullptr,
	    nullptr,
	    &swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	ThrowIfFailed(factory4->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swapChain1.As(&swapChain4));

	CurrentBackBufferIndex = swapChain4->GetCurrentBackBufferIndex();

	return swapChain4;
}

void OWindow::UpdateRenderTargetViews()
{
	auto device = Engine.lock()->GetDevice();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < BuffersCount; i++)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		BackBuffers[i] = backBuffer;
		rtvHandle.Offset(RTVDescriptorSize);
	}
}

HWND OWindow::GetHWND() const
{
	return Hwnd;
}