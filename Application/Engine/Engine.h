#pragma once
#include "../CommandQueue/CommandQueue.h"
#include "../Types/Types.h"
#include "../Window/Window.h"

#include <dxgi1_6.h>

#include <map>

class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	using TWindowPtr = std::shared_ptr<OWindow>;
	using TWindowMap = std::map<HWND, TWindowPtr>;
	using WindowNameMap = std::map<std::wstring, TWindowPtr>;

	static constexpr D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	static constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static constexpr DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	inline static std::map<HWND, TWindowPtr> WindowsMap = {};

	static void RemoveWindow(HWND Hwnd);

	virtual ~OEngine();

	OEngine() = default;

	virtual bool Initialize();

	shared_ptr<OWindow> GetWindow() const;
	Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;

	void FlushGPU() const;
	int Run(shared_ptr<class OTest> Test);

	/**
	 * Get a command queue. Valid types are:
	 * - D3D12_COMMAND_LIST_TYPE_DIRECT : Can be used for draw, dispatch, or copy commands.
	 * - D3D12_COMMAND_LIST_TYPE_COMPUTE: Can be used for dispatch or copy commands.
	 * - D3D12_COMMAND_LIST_TYPE_COPY   : Can be used for copy commands.
	 */

	void DestroyWindow();
	void OnWindowDestroyed();
	bool IsTearingSupported() const;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type) const;
	shared_ptr<OCommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE Type) const;
	void OnEnd(shared_ptr<OTest> Test) const;

	void OnRender(const UpdateEventArgs& Args) const;
	void OnKeyPressed(KeyEventArgs& Args);
	void OnKeyReleased(KeyEventArgs& Args);
	void OnMouseMoved(class MouseMotionEventArgs& Args);
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);
	void OnResize(ResizeEventArgs& Args);

	bool CheckTearingSupport();
	void CreateWindow();
	void CheckMSAAQualitySupport();
	bool GetMSAAState(UINT& Quality) const;
	Microsoft::WRL::ComPtr<IDXGIFactory2> GetFactory() const;

protected:
	shared_ptr<OTest> GetTestByHWND(HWND Handler);
	shared_ptr<OWindow> GetWindowByHWND(HWND Handler);
	void LoadContent();
	void UnloadContent();

	void Destroy();

	shared_ptr<OWindow> Window;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter);

private:
	Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> Device;

	shared_ptr<OCommandQueue> DirectCommandQueue;
	shared_ptr<OCommandQueue> ComputeCommandQueue;
	shared_ptr<OCommandQueue> CopyCommandQueue;

	bool bIsTearingSupported = false;

	bool Msaa4xState = false;
	UINT Msaa4xQuality = 0;

	map<HWND, shared_ptr<OTest>> Tests;
	Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
};
