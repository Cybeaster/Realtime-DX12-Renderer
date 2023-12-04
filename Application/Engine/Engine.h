#pragma once
#include "../CommandQueue/CommandQueue.h"
#include "../Window/Window.h"
#include "..\Types\Types.h"

class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	virtual ~OEngine() = default;

	OEngine(const wstring& _Name, uint32_t _Width, uint32_t _Height, bool _VSync)
	    : Window(make_shared<OWindow>(_Name, _Width, _Height, _VSync))
	{
	}

	virtual bool Initialize();

	shared_ptr<OWindow> GetWindow() const;
	const wstring& GetName() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;

	void FlushGPU();
	shared_ptr<OCommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type);

protected:
	void LoadContent();

	void UnloadContent();

	void Destroy();

	void Update();

	void OnUpdate(UpdateEventArgs& e);

	void OnRender(RenderEventArgs& e);

	void OnResize(ResizeEventArgs& e);

	void OnWindowDestroyed();

protected:
	shared_ptr<OWindow> Window;

private:
	wstring Name;
	uint32_t Width;
	uint32_t Height;
	bool VSync;

	Microsoft::WRL::ComPtr<ID3D12Device2> Device;

	shared_ptr<OCommandQueue> DirectCommandQueue;
	shared_ptr<OCommandQueue> ComputeCommandQueue;
	shared_ptr<OCommandQueue> CopyCommandQueue;
};
