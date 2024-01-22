#pragma once
#include "../../Materials/Material.h"
#include "../../Objects/Geometry/Wave/Waves.h"
#include "../CommandQueue/CommandQueue.h"
#include "../Types/Types.h"
#include "../Window/Window.h"
#include "DirectX/FrameResource.h"
#include "Filters/BlurFilter.h"
#include "RenderItem.h"
#include "ShaderTypes.h"
#include "Textures/Texture.h"

#include <dxgi1_6.h>

#include <map>

class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	using TWindowPtr = std::shared_ptr<OWindow>;
	using TWindowMap = std::map<HWND, TWindowPtr>;
	using WindowNameMap = std::map<std::wstring, TWindowPtr>;
	using TMaterialsMap = std::unordered_map<string, unique_ptr<SMaterial>>;
	using TTexturesMap = std::unordered_map<string, unique_ptr<STexture>>;
	using TSceneGeometryMap = std::unordered_map<string, unique_ptr<SMeshGeometry>>;

	vector<unique_ptr<SFrameResource>> FrameResources;
	SFrameResource* CurrentFrameResources = nullptr;
	UINT CurrentFrameResourceIndex = 0;

	inline static std::map<HWND, TWindowPtr> WindowsMap = {};

	static void RemoveWindow(HWND Hwnd);

	virtual ~OEngine();

	OEngine() = default;

	virtual bool Initialize();
	void PostInitialize();
	shared_ptr<OWindow> GetWindow() const;

	ComPtr<ID3D12Device2> GetDevice() const;

	void FlushGPU() const;

	int InitTests(shared_ptr<class OTest> Test);
	void PostTestInit();
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

	void BuildFrameResource(uint32_t PassCount = 1);

	void OnPreRender();
	void Draw(const UpdateEventArgs& Args);
	void Render(const UpdateEventArgs& Args);
	void Update(const UpdateEventArgs& Args);
	void OnUpdate(const UpdateEventArgs& Args);

	void OnPostRender();
	void PostProcess(HWND Handler);

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

	vector<SRenderItem*>& GetRenderItems(const string& Type);

	void BuildPSOs();
	void BuildBlurPSO();

	void BuildShadersAndInputLayouts();

	D3D12_RENDER_TARGET_BLEND_DESC GetTransparentBlendState();
	ComPtr<IDXGIFactory2> GetFactory() const;

	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;

	std::unordered_map<string, unique_ptr<SMeshGeometry>>& GetSceneGeometry();
	void SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry);
	SMeshGeometry* FindSceneGeometry(const string& Name) const;

	void BuildShaders(const wstring& ShaderPath, const string& VSShaderName, const string& PSShaderName,
	                  const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildShader(const wstring& ShaderPath, const string& ShaderName, const string& ShaderQualifier, const string& ShaderTarget, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildVSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildPSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildGSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildCSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines = nullptr);
	void BuildShader(const wstring& ShaderPath, const string& ShaderName, EShaderLevel ShaderType, std::optional<string> ShaderEntry = {}, const D3D_SHADER_MACRO* Defines = nullptr);

	ComPtr<ID3DBlob> GetShader(const string& ShaderName);

	void CreatePSO(const string& PSOName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc);
	void CreatePSO(const string& PSOName, const D3D12_COMPUTE_PIPELINE_STATE_DESC& PSODesc);
	ComPtr<ID3D12PipelineState> GetPSO(const string& PSOName);

	void AddMaterial(string Name, unique_ptr<SMaterial>& Material);
	void CreateMaterial(const string& Name, int32_t CBIndex, int32_t DiffuseSRVHeapIdx, const SMaterialConstants& Constants);
	const TMaterialsMap& GetMaterials() const;
	SMaterial* FindMaterial(const string& Name) const;

	STexture* CreateTexture(string Name, wstring FileName);
	STexture* FindTexture(string Name) const;

	void AddRenderItem(string Category, unique_ptr<SRenderItem> RenderItem);
	void AddRenderItem(const vector<string>& Categories, unique_ptr<SRenderItem> RenderItem);

	const vector<unique_ptr<SRenderItem>>& GetAllRenderItems();
	void SetPipelineState(string PSOName);

	template<typename... Args>
	void CreateWaves(Args&&... args)
	{
		Waves = std::make_unique<OWaves>(std::forward<Args>(args)...);
	}
	OWaves* GetWaves() const;

	void SetFog(DirectX::XMFLOAT4 Color, float Start, float Range);
	SPassConstants& GetMainPassCB();
	ComPtr<ID3D12DescriptorHeap>& GetSRVHeap();

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	ComPtr<ID3D12RootSignature> DefaultRootSignature = nullptr;
	OBlurFilter* GetBlurFilter();

protected:
	shared_ptr<OTest> GetTestByHWND(HWND Handler);
	shared_ptr<OWindow> GetWindowByHWND(HWND Handler);
	void LoadContent();
	void UnloadContent();
	void Destroy();

	shared_ptr<OWindow> Window;
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> Adapter);
	void BuildPostProcessRootSignature();
	void BuildDefaultRootSignature();

	ComPtr<ID3D12Resource> GetBackBuffer() const;

private:
	void UpdateMainPass(const STimer& Timer);
	SPassConstants MainPassCB;

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

	map<string, vector<SRenderItem*>> RenderItems;
	vector<unique_ptr<SRenderItem>> AllRenderItems;

	std::unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	std::unordered_map<string, ComPtr<ID3D12PipelineState>> PSOs;

	TSceneGeometryMap SceneGeometry;
	TTexturesMap Textures;
	TMaterialsMap Materials;

	unique_ptr<OWaves> Waves = nullptr;

	ComPtr<ID3D12RootSignature> PostProcessRootSignature = nullptr;

	unique_ptr<OBlurFilter> BlurFilter = nullptr;
	ComPtr<ID3D12DescriptorHeap> SRVHeap;
	bool HasInitializedTests = false;
};
