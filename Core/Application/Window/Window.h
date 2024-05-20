#pragma once
#include "Engine/RenderTarget/RenderTarget.h"
#include "Events.h"
#include "InputHandler/InputHandler.h"

#include <DirectXMath.h>
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

class OWindow : public ORenderTargetBase
{
public:
	virtual ~OWindow() = default;

	OWindow() = default;

	OWindow(HWND hWnd, const SWindowInfo& InWindowInfo);
	void InitRenderObject();
	wstring GetName() const override;
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
	SResourceInfo* GetCurrentBackBuffer();
	SResourceInfo* GetCurrentDepthStencilBuffer();
	void UpdateCameraClip(float NewFar);
	void SetVSync(bool vSync);
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
	void BuildResource() override {} // for now do nothing
	HWND GetHWND() const;
	uint64_t FenceValues[SRenderConstants::RenderBuffersCount];

	TResourceInfo DepthBuffer;

	void MoveToNextFrame();
	const ComPtr<IDXGISwapChain4>& GetSwapChain();

	void TransitionResource(ComPtr<ID3D12GraphicsCommandList> CommandList, Microsoft::WRL::ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState);
	void ClearRTV(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE RTV, FLOAT* ClearColor);
	void ClearDepth(ComPtr<ID3D12GraphicsCommandList> CommandList, D3D12_CPU_DESCRIPTOR_HANDLE DSV, FLOAT Depth = 1.0f);

	// Update and Draw can only be called by the application.
	virtual void OnRender(const UpdateEventArgs& Event);
	virtual void Update(const UpdateEventArgs& Event) override;

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
	weak_ptr<OCamera> GetCamera();

	void ResetBuffers();
	void SetCameraLens();

	uint32_t GetNumDSVRequired() const override;
	uint32_t GetNumRTVRequired() const override;
	SDescriptorPair GetRTV(uint32_t SubtargetIdx = 0) const override;
	SDescriptorPair GetDSV(uint32_t SubtargetIdx = 0) const override;
	uint32_t GetNumSRVRequired() const override;
	SDescriptorPair GetSRVDepth() const;
	SResourceInfo* GetResource() override;

protected:
	void BuildEssentials();
	void BuildDescriptors(IDescriptor* Descriptor) override;
	bool HasCapturedLeftMouseButton() const;
	// The Window procedure needs to call protected methods of this class.
	friend LRESULT CALLBACK
	WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Create the swapchian.
	ComPtr<IDXGISwapChain4> CreateSwapChain();

	void UpdateRenderTargetViews();
	void ResizeDepthBuffer();

public:
	void BuildDescriptors() override;

private:
	array<SDescriptorPair, SRenderConstants::RenderBuffersCount> RTVHandles;
	SDescriptorPair DSVHandle;
	SDescriptorPair SRVDepthHandle;

	shared_ptr<OInputHandler> InputHandler;
	shared_ptr<OCamera> Camera;

	HWND Hwnd = nullptr;

	UINT CurrentBackBufferIndex;
	RECT WindowRect;

	uint64_t FrameCounter = 0;

	ComPtr<IDXGISwapChain4> SwapChain;
	TResourceInfo BackBuffers[SRenderConstants::RenderBuffersCount];

	SWindowInfo WindowInfo;

	float LastMouseXPos = 0;
	float LastMouseYPos = 0;
	float LeftButtonPressedTime = 0;
	float CameraLeftMoveTime = 0.5;
};
