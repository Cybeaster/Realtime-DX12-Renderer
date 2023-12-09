#pragma once
#include "../CommandQueue/CommandQueue.h"
#include "../Window/Window.h"
#include "..\Types\Types.h"

#include <dxgi1_6.h>

#include <map>

class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	using TWindowPtr = std::shared_ptr<OWindow>;
	using TWindowMap = std::map<HWND, TWindowPtr>;
	using WindowNameMap = std::map<std::wstring, TWindowPtr>;
	inline static std::map<HWND, TWindowPtr> WindowsMap = {};
	static void RemoveWindow(HWND Hwnd);

	virtual ~OEngine() = default;

	OEngine(const wstring& _Name, uint32_t _Width, uint32_t _Height, bool _VSync)
	    : Name(_Name), Width(_Width), Height(_Height), bVSync(_VSync)
	{
	}

	virtual bool Initialize();

	shared_ptr<OWindow> GetWindow() const;
	const wstring& GetName() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	bool IsVSync() const;
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

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type);
	shared_ptr<OCommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

	void Update();
	void OnEnd(shared_ptr<OTest> Test) const;
	void OnUpdate(UpdateEventArgs& e);
	void OnRender(RenderEventArgs& e);
	void OnResize(ResizeEventArgs& e);

	void OnKeyPressed(KeyEventArgs& Args);
	void OnKeyReleased(KeyEventArgs& Args);
	void OnMouseMoved(class MouseMotionEventArgs& Args);
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);

	bool CheckTearingSupport();
	void CreateWindow();
	void InitWindowClass();

protected:
	void LoadContent();
	void UnloadContent();

	void Destroy();

	shared_ptr<OWindow> Window;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter);

private:
	wstring Name;
	uint32_t Width;
	uint32_t Height;
	bool bVSync;

	HINSTANCE HInstance = nullptr;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> Device;

	shared_ptr<OCommandQueue> DirectCommandQueue;
	shared_ptr<OCommandQueue> ComputeCommandQueue;
	shared_ptr<OCommandQueue> CopyCommandQueue;

	bool bIsTearingSupported = false;
};
