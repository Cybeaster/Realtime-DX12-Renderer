#pragma once
#include "../../Utils/Math.h"
#include "../InputHandler/InputHandler.h"
#include "Events.h"

#include <DirectXMath.h>
#include <Types.h>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>

#if defined(CreateWindow)
#undef CreateWindow
#endif

class OCamera;
struct SWindowInfo
{
	bool Fullscreen = false;
	wstring Name = L"NONE";
	uint32_t ClientWidth = 1920;
	uint32_t ClientHeight = 1080;
	bool VSync = false;
	float FoV = 45.0f;
};

class OEngine;

class OWindow : public std::enable_shared_from_this<OWindow>
{
public:
	virtual ~OWindow() = default;
	static constexpr uint32_t BuffersCount = 2;

	OWindow() = default;

	OWindow(HWND hWnd, const SWindowInfo& _WindowInfo);
	void Init();
	const wstring& GetName() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	UINT GetCurrentBackBufferIndex() const;

	/**
	 * Present the swapchain's back buffer to the screen.
	 * Returns the current back buffer index after the present.
	 */
	UINT Present();

	/**
	 * Get the render target view for the current back buffer.
	 */
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

	/**
	 * Get the back buffer resource for the current back buffer.
	 */
	Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

	/**
	 * Should this window be rendered with vertical refresh synchronization.
	 */
	bool IsVSync() const;
	void SetVSync(bool vSync);
	void ToggleVSync();
	float GetFoV();
	void SetFoV(float _FoV);

	float GetAspectRatio() const;
	/**
	 * Is this a windowed window or full-screen?
	 */
	bool IsFullScreen() const;

	// Set the fullscreen state of the window.
	void SetFullscreen(bool _Fullscreen);
	void ToggleFullscreen();

	/**
	 * Show this window.
	 */
	void Show();

	/**
	 * Hide the window.
	 */
	void Hide();
	void RegsterWindow();
	void Destroy();

	HWND GetHWND() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStensilView() const;

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;
	uint64_t FenceValues[BuffersCount];

	ComPtr<ID3D12Resource> DepthBuffer;
	ComPtr<ID3D12DescriptorHeap> DSVHeap;
	ComPtr<ID3D12DescriptorHeap> RTVHeap;

	void MoveToNextFrame();
	const ComPtr<IDXGISwapChain4>& GetSwapChain();

	void TransitionResource(ComPtr<ID3D12GraphicsCommandList> CommandList, Microsoft::WRL::ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState);
	void ClearRTV(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor);
	void ClearDepth(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth = 1.0f);

	// Update and Draw can only be called by the application.
	virtual void OnRender(const UpdateEventArgs& Event);
	virtual void OnUpdate(const UpdateEventArgs& Event);

	// A keyboard key was pressed
	virtual void OnKeyPressed(KeyEventArgs& Event);
	// A keyboard key was released
	virtual void OnKeyReleased(KeyEventArgs& Event);

	// The mouse was moved
	virtual void OnMouseMoved(MouseMotionEventArgs& Event);
	// A button on the mouse was pressed
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& Event);
	// A button on the mouse was released
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& Event);
	// The mouse wheel was moved.
	virtual void OnMouseWheel(MouseWheelEventArgs& Event);

	// The window was resized.
	virtual void OnResize(ResizeEventArgs& Event);
	void OnUpdateWindowSize(ResizeEventArgs& Event);
	float GetLastXMousePos() const;
	float GetLastYMousePos() const;
	shared_ptr<OCamera> GetCamera();

protected:
	// The Window procedure needs to call protected methods of this class.
	friend LRESULT CALLBACK
	WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Create the swapchian.
	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

	void UpdateRenderTargetViews();
	void ResizeDepthBuffer();

private:
	shared_ptr<OInputHandler> InputHandler;
	shared_ptr<OCamera> Camera;

	HWND Hwnd = nullptr;

	UINT CurrentBackBufferIndex;
	UINT RTVDescriptorSize;
	RECT WindowRect;

	uint64_t FrameCounter = 0;

	ComPtr<IDXGISwapChain4> SwapChain;
	ComPtr<ID3D12Resource> BackBuffers[BuffersCount];

	SWindowInfo WindowInfo;

	float LastMouseXPos = 0;
	float LastMouseYPos = 0;
};
