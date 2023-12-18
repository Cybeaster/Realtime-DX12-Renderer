//
// Created by Cybea on 02/12/2023.
//

#include "Window.h"

#include "../../Utils/MathUtils.h"
#include "Application.h"
#include "Exception.h"
#include "Logger.h"

#include <DXHelper.h>
#include <Types.h>
#pragma optimize("", off)
using namespace Microsoft::WRL;
OWindow::OWindow(shared_ptr<OEngine> _Engine, HWND hWnd, const SWindowInfo& _WindowInfo)
    : Hwnd(hWnd), Engine(_Engine), WindowInfo{ _WindowInfo }
{
	IsTearingSupported = _Engine->IsTearingSupported();

	SwapChain = CreateSwapChain();
	RTVDescriptorHeap = _Engine->CreateDescriptorHeap(BuffersCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	RTVDescriptorSize = _Engine->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	UpdateRenderTargetViews();
	ResizeDepthBuffer();
}
const wstring& OWindow::GetName() const
{
	return WindowInfo.Name;
}

uint32_t OWindow::GetWidth() const
{
	return WindowInfo.ClientWidth;
}

uint32_t OWindow::GetHeight() const
{
	return WindowInfo.ClientHeight;
}

UINT OWindow::GetCurrentBackBufferIndex() const
{
	return CurrentBackBufferIndex;
}

UINT OWindow::Present()
{
	UINT syncInterval = WindowInfo.VSync ? 1 : 0;
	UINT presentFlags = IsTearingSupported && !WindowInfo.VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;

	THROW_IF_FAILED(SwapChain->Present(syncInterval, presentFlags));
	CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	return CurrentBackBufferIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE OWindow::GetCurrentRenderTargetView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	                                     CurrentBackBufferIndex,
	                                     RTVDescriptorSize);
}

ComPtr<ID3D12Resource> OWindow::GetCurrentBackBuffer() const
{
	return BackBuffers[CurrentBackBufferIndex];
}

bool OWindow::IsVSync() const
{
	return WindowInfo.VSync;
}

void OWindow::SetVSync(bool vSync)
{
	WindowInfo.VSync = vSync;
}

void OWindow::ToggleVSync()
{
	SetVSync(!WindowInfo.VSync);
}
float OWindow::GetFoV()
{
	return WindowInfo.FoV;
}

void OWindow::SetFoV(float _FoV)
{
	WindowInfo.FoV = _FoV;
}

float OWindow::GetAspectRatio() const
{
	return static_cast<float>(WindowInfo.ClientWidth) / static_cast<float>(WindowInfo.ClientHeight);
}

bool OWindow::IsFullScreen() const
{
	return WindowInfo.Fullscreen;
}

void OWindow::SetFullscreen(bool _Fullscreen)
{
	if (WindowInfo.Fullscreen != _Fullscreen)
	{
		WindowInfo.Fullscreen = _Fullscreen;
		if (WindowInfo.Fullscreen)
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
	SetFullscreen(!WindowInfo.Fullscreen);
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

void OWindow::OnRender(const UpdateEventArgs& Event)
{
}

void OWindow::OnKeyPressed(KeyEventArgs& Event)
{
}

void OWindow::OnKeyReleased(KeyEventArgs& Event)
{
}

void OWindow::OnMouseMoved(MouseMotionEventArgs& Event)
{
	LastMouseXPos = Event.X;
	LastMouseYPos = Event.Y;
}

void OWindow::OnMouseButtonPressed(MouseButtonEventArgs& Event)
{
	LastMouseXPos = Event.X;
	LastMouseYPos = Event.Y;
}

void OWindow::OnMouseButtonReleased(MouseButtonEventArgs& Event)
{
}

void OWindow::OnMouseWheel(MouseWheelEventArgs& Event)
{
}

void OWindow::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> CommandList, ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState)
{
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), BeforeState, AfterState);
	CommandList->ResourceBarrier(1, &barrier);
}

void OWindow::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor)
{
	CommandList->ClearRenderTargetView(RTV, ClearColor, 0, nullptr);
}

void OWindow::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth)
{
	CommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, Depth, 0, 0, nullptr);
}

void OWindow::OnResize(ResizeEventArgs& Event)
{
	if (WindowInfo.ClientWidth != Event.Width || WindowInfo.ClientHeight != Event.Height)
	{
		WindowInfo.ClientWidth = std::max(1, Event.Width);
		WindowInfo.ClientHeight = std::max(1, Event.Height);

		Engine.lock()->FlushGPU();

		for (int i = 0; i < BuffersCount; ++i)
		{
			// Flush any GPU commands that might be referencing the back buffers
			BackBuffers[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		THROW_IF_FAILED(SwapChain->GetDesc(&swapChainDesc));
		THROW_IF_FAILED(SwapChain->ResizeBuffers(BuffersCount, WindowInfo.ClientWidth, WindowInfo.ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
		Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Event.Width), static_cast<float>(Event.Height));

		UpdateRenderTargetViews();
		ResizeDepthBuffer();
	}
}

float OWindow::GetLastXMousePos() const
{
	return LastMouseXPos;
}

float OWindow::GetLastYMousePos() const
{
	return LastMouseYPos;
}

shared_ptr<OCamera> OWindow::GetCamera()
{
	return Camera;
}

ComPtr<IDXGISwapChain4> OWindow::CreateSwapChain()
{
	const auto engine = Engine.lock();
	ComPtr<IDXGISwapChain4> swapChain4;
	ComPtr<IDXGIFactory4> factory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	THROW_IF_FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory4)));
	UINT msaaQuality;
	bool msaaState = engine->GetMSAAState(msaaQuality);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = WindowInfo.ClientWidth;
	swapChainDesc.Height = WindowInfo.ClientHeight;
	swapChainDesc.Format = Engine.lock()->BackBufferFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = msaaState ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = msaaState ? msaaQuality - 1 : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BuffersCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = IsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	const auto commandQueue = engine->GetCommandQueue()->GetCommandQueue();

	ComPtr<IDXGISwapChain1> swapChain1;
	THROW_IF_FAILED(factory4->CreateSwapChainForHwnd(
	    commandQueue.Get(),
	    Hwnd,
	    &swapChainDesc,
	    nullptr,
	    nullptr,
	    &swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	THROW_IF_FAILED(factory4->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_ALT_ENTER));
	THROW_IF_FAILED(swapChain1.As(&swapChain4));

	CurrentBackBufferIndex = swapChain4->GetCurrentBackBufferIndex();

	return swapChain4;
}

void OWindow::UpdateRenderTargetViews()
{
	const auto device = Engine.lock()->GetDevice();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < BuffersCount; i++)
	{
		ComPtr<ID3D12Resource> backBuffer;
		THROW_IF_FAILED(SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		BackBuffers[i] = backBuffer;
		rtvHandle.Offset(RTVDescriptorSize);
	}
}
void OWindow::ResizeDepthBuffer()
{
	// Flush any GPU commands that might be referencing the depth buffer.
	const auto engine = Engine.lock();
	engine->FlushGPU();
	WindowInfo.ClientWidth = std::max(static_cast<uint32_t>(1), WindowInfo.ClientWidth);
	WindowInfo.ClientHeight = std::max(static_cast<uint32_t>(1), WindowInfo.ClientHeight);

	const auto device = engine->GetDevice();

	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	// Create named instances for heap properties and resource description
	const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, WindowInfo.ClientWidth, WindowInfo.ClientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	THROW_IF_FAILED(device->CreateCommittedResource(
	    &heapProperties,
	    D3D12_HEAP_FLAG_NONE,
	    &resourceDesc,
	    D3D12_RESOURCE_STATE_DEPTH_WRITE,
	    &optimizedClearValue,
	    IID_PPV_ARGS(&DepthBuffer)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	THROW_IF_FAILED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&DSVDescriptorHeap)));

	// Depth stencil view description
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(DepthBuffer.Get(), &dsvDesc, DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

HWND OWindow::GetHWND() const
{
	return Hwnd;
}

ComPtr<ID3D12DescriptorHeap> OWindow::GetDSVDescriptorHeap() const
{
	return DSVDescriptorHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE OWindow::GetDepthStensilView() const
{
	return DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

#pragma optimize("", on)