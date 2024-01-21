//
// Created by Cybea on 02/12/2023.
//

#include "Window.h"

#include "..\..\Utils\Math.h"
#include "Application.h"
#include "Camera/Camera.h"
#include "Exception.h"
#include "Logger.h"
#include "RenderConstants.h"

#include <DXHelper.h>
#include <Types.h>
#pragma optimize("", off)
using namespace Microsoft::WRL;

OWindow::OWindow(shared_ptr<OEngine> _Engine, HWND hWnd, const SWindowInfo& _WindowInfo, const shared_ptr<OCamera>& _Camera)
    : Hwnd(hWnd)
    , Engine(_Engine)
    , WindowInfo{ _WindowInfo }
    , Camera(_Camera)
{
	SwapChain = CreateSwapChain();
	RTVDescriptorHeap = _Engine->CreateDescriptorHeap(BuffersCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	RTVDescriptorSize = _Engine->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(Engine.lock()->GetDevice()->CreateDescriptorHeap(
	    &dsvHeapDesc, IID_PPV_ARGS(DSVDescriptorHeap.GetAddressOf())));

	UpdateRenderTargetViews();
	//ResizeDepthBuffer();
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
	THROW_IF_FAILED(SwapChain->Present(0, 0));
	CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	return CurrentBackBufferIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE OWindow::CurrentBackBufferView() const
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
	Show();
	UpdateWindow(Hwnd);
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

void OWindow::OnUpdate(const UpdateEventArgs& Event)
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
	SetCapture(Hwnd);
}

void OWindow::OnMouseButtonReleased(MouseButtonEventArgs& Event)
{
	ReleaseCapture();
}

void OWindow::OnMouseWheel(MouseWheelEventArgs& Event)
{
}

void OWindow::MoveToNextFrame()
{
	CurrentBackBufferIndex = (CurrentBackBufferIndex + 1) % BuffersCount;
}

const ComPtr<IDXGISwapChain4>& OWindow::GetSwapChain()
{
	return SwapChain;
}

void OWindow::TransitionResource(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState)
{
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), BeforeState, AfterState);
	CommandList->ResourceBarrier(1, &barrier);
}

void OWindow::ClearRTV(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor)
{
	CommandList->ClearRenderTargetView(RTV, ClearColor, 0, nullptr);
}

void OWindow::ClearDepth(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth)
{
	CommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, Depth, 0, 0, nullptr);
}

void OWindow::OnUpdateWindowSize(ResizeEventArgs& Event)
{
	WindowInfo.ClientHeight = Event.Height;
	WindowInfo.ClientWidth = Event.Width;
}

void OWindow::OnResize(ResizeEventArgs& Event)
{
	const auto engine = Engine.lock();
	engine->FlushGPU();
	engine->GetCommandQueue()->ResetCommandList();

	for (int i = 0; i < BuffersCount; ++i)
	{
		// Flush any GPU commands that might be referencing the back buffers
		BackBuffers[i].Reset();
	}
	DepthBuffer.Reset();

	THROW_IF_FAILED(SwapChain->ResizeBuffers(BuffersCount, WindowInfo.ClientWidth, WindowInfo.ClientHeight, SRenderConstants::BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	CurrentBackBufferIndex = 0;

	UpdateRenderTargetViews();
	ResizeDepthBuffer();

	auto list = engine->GetCommandQueue()->GetCommandList();
	list->Close();
	ID3D12CommandList* cmdsLists[] = { list.Get() };
	engine->GetCommandQueue()->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	engine->FlushGPU();

	Viewport = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<float>(WindowInfo.ClientWidth), static_cast<float>(WindowInfo.ClientHeight), 0.0f, 1.0f);
	ScissorRect = CD3DX12_RECT(0, 0, WindowInfo.ClientWidth, WindowInfo.ClientHeight);
	XMStoreFloat4x4(&ProjectionMatrix, DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, GetAspectRatio(), 1.0f, 1000.0f));
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
	UINT msaaQuality;
	const bool msaaState = engine->GetMSAAState(msaaQuality);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = WindowInfo.ClientWidth;
	swapChainDesc.Height = WindowInfo.ClientHeight;
	swapChainDesc.Format = SRenderConstants::BackBufferFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = msaaState ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = msaaState ? msaaQuality - 1 : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BuffersCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	const auto commandQueue = engine->GetCommandQueue()->GetCommandQueue();

	ComPtr<IDXGISwapChain1> swapChain1;
	THROW_IF_FAILED(engine->GetFactory()->CreateSwapChainForHwnd(
	    commandQueue.Get(),
	    Hwnd,
	    &swapChainDesc,
	    nullptr,
	    nullptr,
	    &swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	THROW_IF_FAILED(engine->GetFactory()->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_ALT_ENTER));
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
		THROW_IF_FAILED(SwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffers[i])));
		device->CreateRenderTargetView(BackBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(RTVDescriptorSize);
	}
}

void OWindow::ResizeDepthBuffer()
{
	// Flush any GPU commands that might be referencing the depth buffer.
	const auto engine = Engine.lock();
	engine->FlushGPU();

	auto list = engine->GetCommandQueue()->GetCommandList();
	UINT quality;
	auto mxaaEnabled = engine->GetMSAAState(quality);
	DepthBuffer.Reset();
	const auto device = engine->GetDevice();
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = GetWidth();
	depthStencilDesc.Height = GetHeight();
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = mxaaEnabled ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = mxaaEnabled ? (quality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_CLEAR_VALUE optClear;
	optClear.Format = SRenderConstants::DepthBufferFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	THROW_IF_FAILED(device->CreateCommittedResource(
	    &defaultHeap,
	    D3D12_HEAP_FLAG_NONE,
	    &depthStencilDesc,
	    D3D12_RESOURCE_STATE_COMMON,
	    &optClear,
	    IID_PPV_ARGS(DepthBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = SRenderConstants::DepthBufferFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(DepthBuffer.Get(), &dsvDesc, GetDepthStensilView());

	TransitionResource(engine->GetCommandQueue()->GetCommandList(), DepthBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
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
