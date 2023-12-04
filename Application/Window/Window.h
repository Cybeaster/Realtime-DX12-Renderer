#pragma once
#include "../InputHandler/InputHandler.h"
#include "Events.h"

#include <Types.h>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>

#if defined(CreateWindow)
#undef CreateWindow
#endif

class OEngine;

class OWindow
{
public:
	static constexpr uint32_t BuffersCount = 3;

	OWindow() = default;

	OWindow(const wstring& _Name, uint32_t _Width, uint32_t _Height, bool _VSync)
	    : Name(_Name), Width(_Width), Height(_Height), VSync(_VSync)
	{
	}

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
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

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

	/**
	 * Is this a windowed window or full-screen?
	 */
	bool IsFullScreen() const;

	// Set the fullscreen state of the window.
	void SetFullscreen(bool fullscreen);
	void ToggleFullscreen();

	/**
	 * Show this window.
	 */
	void Show();

	/**
	 * Hide the window.
	 */
	void Hide();

protected:
	// The Window procedure needs to call protected methods of this class.
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Update and Draw can only be called by the application.
	virtual void OnUpdate(UpdateEventArgs& Event);
	virtual void OnRender(RenderEventArgs& Event);

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

	// Create the swapchian.
	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

private:
	shared_ptr<OInputHandler> InputHandler;

	HWND Hwnd;

	UINT CurrentBackBufferIndex;
	UINT RTVDescriptorSize;
	RECT WindowRect;

	uint64_t FrameCounter;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTVDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> BackBuffers[BuffersCount];

	bool IsFullscreen;
	wstring Name;
	uint32_t Width;
	uint32_t Height;
	bool VSync;
};
