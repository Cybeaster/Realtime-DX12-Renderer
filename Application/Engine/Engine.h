#pragma once
#include "../CommandQueue/CommandQueue.h"
#include "../Types/Types.h"
#include "../Window/Window.h"
#include "DirectX/FrameResource.h"
#include "RenderItem.h"
#include "../../Materials/Material.h"
#include "../../Objects/Geometry/Wave/Waves.h"

#include <dxgi1_6.h>

#include <map>


class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	using TWindowPtr = std::shared_ptr<OWindow>;
	using TWindowMap = std::map<HWND, TWindowPtr>;
	using WindowNameMap = std::map<std::wstring, TWindowPtr>;
	using TMaterialsMap = std::unordered_map<string, unique_ptr<SMaterial>>;

	vector<unique_ptr<SFrameResource>> FrameResources;
	SFrameResource* CurrentFrameResources = nullptr;
	UINT CurrentFrameResourceIndex = 0;

	inline static std::map<HWND, TWindowPtr> WindowsMap = {};

	static void RemoveWindow(HWND Hwnd);

	virtual ~OEngine();

	OEngine() = default;

	virtual bool Initialize();

	shared_ptr<OWindow> GetWindow() const;

	ComPtr<ID3D12Device2> GetDevice() const;

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

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

	shared_ptr<OCommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);

	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

	void OnEnd(shared_ptr<OTest> Test) const;

	void BuildFrameResource();

	void OnRender(const UpdateEventArgs& Args) const;

	void OnKeyPressed(KeyEventArgs& Args);

	void OnKeyReleased(KeyEventArgs& Args);

	void OnMouseMoved(class MouseMotionEventArgs& Args);

	void OnMouseButtonPressed(MouseButtonEventArgs& Args);

	void OnMouseButtonReleased(MouseButtonEventArgs& Args);

	void OnMouseWheel(MouseWheelEventArgs& Args);

	void OnResize(ResizeEventArgs& Args);

	void OnUpdateWindowSize(ResizeEventArgs& Args);

	bool CheckTearingSupport();

	void CreateWindow();

	void CheckMSAAQualitySupport();

	bool GetMSAAState(UINT& Quality) const;

	vector<unique_ptr<SRenderItem>>& GetRenderItems();

	vector<SRenderItem*>& GetOpaqueRenderItems();

	vector<unique_ptr<SRenderItem>>& GetTransparentRenderItems();

	ComPtr<IDXGIFactory2> GetFactory() const;

	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;

	std::unordered_map<string, unique_ptr<SMeshGeometry>>& GetSceneGeometry();

	void SetSceneGeometry(const string& Name, unique_ptr<SMeshGeometry> Geometry);

	void BuildShader(const wstring& ShaderName, const string& VSShaderName, const string& PSShaderName,
	                 const D3D_SHADER_MACRO* Defines = nullptr);

	ComPtr<ID3DBlob> GetShader(const string& ShaderName);

	void BuildPSO(const string& PSOName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc);

	ComPtr<ID3D12PipelineState> GetPSO(const string& PSOName);

	OWaves* GetWaves() const;

	template<typename... Args>
	void CreateWaves(Args&&... args)
	{
		Waves = std::make_unique<OWaves>(std::forward<Args>(args)...);
	}

	void AddMaterial(string Name, unique_ptr<SMaterial>& Material);

	const TMaterialsMap& GetMaterials() const;
	SMaterial* FindMaterial(const string& Name) const;
protected:
	shared_ptr<OTest> GetTestByHWND(HWND Handler);

	shared_ptr<OWindow> GetWindowByHWND(HWND Handler);

	void LoadContent();

	void UnloadContent();

	void Destroy();

	shared_ptr<OWindow> Window;

	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);

	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> Adapter);

private:
	ComPtr<IDXGIAdapter4> Adapter;
	ComPtr<ID3D12Device2> Device;

	shared_ptr<OCommandQueue> DirectCommandQueue;
	shared_ptr<OCommandQueue> ComputeCommandQueue;
	shared_ptr<OCommandQueue> CopyCommandQueue;

	bool bIsTearingSupported = false;

	bool Msaa4xState = false;
	UINT Msaa4xQuality = 0;

	map<HWND, shared_ptr<OTest>> Tests;
	ComPtr<IDXGIFactory4> Factory;

	vector<unique_ptr<SRenderItem>> AllRenderItems;
	vector<SRenderItem*> OpaqueRenderItems;
	vector<unique_ptr<SRenderItem>> TransparentRenderItems;

	std::unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	std::unordered_map<string, ComPtr<ID3D12PipelineState>> PSOs;
	std::unordered_map<string, unique_ptr<SMeshGeometry>> SceneGeometry;

	TMaterialsMap Materials;

	unique_ptr<OWaves> Waves = nullptr;
};
