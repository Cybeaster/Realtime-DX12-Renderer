#pragma once
#include "../../Materials/Material.h"
#include "../../Objects/Geometry/Wave/Waves.h"
#include "../CommandQueue/CommandQueue.h"
#include "../Types/Types.h"
#include "../Window/Window.h"
#include "DirectX/FrameResource.h"
#include "Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Filters/Blur/BlurFilter.h"
#include "Filters/SobelFilter/SobelFilter.h"
#include "RenderItem.h"
#include "RenderTarget/RenderTarget.h"
#include "ShaderTypes.h"
#include "Textures/Texture.h"
#include "UI/UIManager/UiManager.h"

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
	using TRenderLayer = map<string, vector<SRenderItem*>>;
	vector<unique_ptr<SFrameResource>> FrameResources;
	SFrameResource* CurrentFrameResources = nullptr;
	UINT CurrentFrameResourceIndex = 0;

	inline static std::map<HWND, TWindowPtr> WindowsMap = {};

	static void RemoveWindow(HWND Hwnd);

	virtual ~OEngine();

	OEngine() = default;

	virtual bool Initialize();
	void PostInitialize();
	void InitUIManager();

	void SetFogColor(DirectX::XMFLOAT4 Color);
	void SetFogStart(float Start);
	void SetFogRange(float Range);

	shared_ptr<OWindow> GetWindow() const;
	ComPtr<ID3D12Device2> GetDevice() const;

	void FlushGPU() const;

	int InitTests(shared_ptr<class OTest> Test);
	void PostTestInit();

	void DestroyWindow();
	void OnWindowDestroyed();
	bool IsTearingSupported() const;

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

	shared_ptr<OCommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);

	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

	void OnEnd(shared_ptr<OTest> Test) const;

	void BuildFrameResource(uint32_t PassCount = 1);

	void OnPreRender();
	void Draw(UpdateEventArgs& Args);
	void Render(UpdateEventArgs& Args);
	void Update(UpdateEventArgs& Args);
	void OnUpdate(UpdateEventArgs& Args);

	void OnPostRender();
	void PostProcess(HWND Handler);
	void DrawCompositeShader(CD3DX12_GPU_DESCRIPTOR_HANDLE Input);
	void OnKeyPressed(KeyEventArgs& Args);
	void OnKeyReleased(KeyEventArgs& Args);
	void OnMouseMoved(class MouseMotionEventArgs& Args);
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);
	void OnResizeRequest(HWND& WindowHandle);
	void OnUpdateWindowSize(ResizeEventArgs& Args);

	bool CheckTearingSupport();

	void CreateWindow();

	void CheckMSAAQualitySupport();

	bool GetMSAAState(UINT& Quality) const;

	vector<SRenderItem*>& GetRenderItems(const string& Type);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetOpaquePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetAlphaTestedPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetTransparentPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetShadowPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetReflectedPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetMirrorPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDebugPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetCompositePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetTreeSpritePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetIcosahedronPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetWavesRenderPSODesc();

	D3D12_COMPUTE_PIPELINE_STATE_DESC GetBilateralBlurPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetWavesDisturbPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetWavesUpdatePSODesc();

	D3D12_COMPUTE_PIPELINE_STATE_DESC GetSobelPSODesc();

	void BuildDescriptorHeap();
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

	void SetFog(DirectX::XMFLOAT4 Color, float Start, float Range);
	SPassConstants& GetMainPassCB();
	ComPtr<ID3D12DescriptorHeap>& GetSRVHeap();

	ID3D12RootSignature* GetDefaultRootSignature() const;
	ID3D12RootSignature* GetWavesRootSignature() const;

	vector<D3D12_INPUT_ELEMENT_DESC>& GetDefaultInputLayout();

	OBlurFilter* GetBlurFilter();
	OBilateralBlurFilter* GetBilateralBlurFilter();
	OSobelFilter* GetSobelFilter();

	void BuildFilters();

	template<typename T, typename... Args>
	T* BuildRenderObject(Args&&... Params);

	TUUID AddRenderObject(IRenderObject* RenderObject);

	void BuildOffscreenRT();
	ORenderTarget* GetOffscreenRT() const;
	void DrawFullScreenQuad();
	uint32_t GetTextureNum();

	template<typename T>
	TUUID AddFilter();

	template<typename T>
	T* GetObjectByUUID(TUUID UUID, bool Checked = false);

	SRenderObjectDescriptor GetObjectDescriptor();
	void SetLightSources(const vector<SLight>& Lights);
	void SetAmbientLight(const DirectX::XMFLOAT3& Color);
	void RebuildGeometry(string Name);

protected:
	template<typename T, typename... Args>
	TUUID BuildRenderObjectImpl(Args&&... Params);

	shared_ptr<OTest> GetTestByHWND(HWND Handler);
	shared_ptr<OWindow> GetWindowByHWND(HWND Handler);
	void Destroy();

	shared_ptr<OWindow> Window;
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> Adapter);

	void BuildPostProcessRootSignature();
	void BuildDefaultRootSignature();
	void BuildBlurRootSignature();
	void BuildWavesRootSignature();
	void BuildBilateralBlurRootSignature();

	ComPtr<ID3D12Resource> GetBackBuffer() const;
	uint32_t GetNumOffscrenRT() const;

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

	TRenderLayer RenderLayers;
	vector<unique_ptr<SRenderItem>> AllRenderItems;

	std::unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	std::unordered_map<string, ComPtr<ID3D12PipelineState>> PSOs;

	TSceneGeometryMap SceneGeometry;
	TTexturesMap Textures;
	TMaterialsMap Materials;

	ComPtr<ID3D12RootSignature> PostProcessRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> WavesRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> DefaultRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> BlurRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> BilateralBlurRootSignature = nullptr;

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	//filters
	TUUID BlurFilterUUID;
	TUUID SobelFilterUUID;
	TUUID BilateralFilterUUID;

	ComPtr<ID3D12DescriptorHeap> SRVDescriptorHeap;
	SRenderObjectDescriptor SRVDescriptor;

	bool HasInitializedTests = false;

	STimer TickTimer;

	ORenderTarget* OffscreenRT = nullptr;
	unique_ptr<OUIManager> UIManager;

	map<TUUID, unique_ptr<IRenderObject>> RenderObjects;

	int32_t LightCount = 0;
};

template<typename T, typename... Args>
TUUID OEngine::BuildRenderObjectImpl(Args&&... Params)
{
	auto object = new T(std::forward<Args>(Params)...);
	auto uuid = GenerateUUID();
	RenderObjects[uuid] = unique_ptr<IRenderObject>(object);
	return uuid;
}

template<typename T, typename... Args>
T* OEngine::BuildRenderObject(Args&&... Params)
{
	return GetObjectByUUID<T>(BuildRenderObjectImpl<T>(std::forward<Args>(Params)...));
}
template<typename T>
T* OEngine::GetObjectByUUID(TUUID UUID, bool Checked)
{
	if (!RenderObjects.contains(UUID))
	{
		if (Checked)
		{
			LOG(Engine, Error, "Render object not found!");
		}
		return nullptr;
	}
	const auto obj = RenderObjects[UUID].get();
	return Cast<T>(obj);
}

template<typename T>
TUUID OEngine::AddFilter()
{
	return AddRenderObject(OFilterBase::CreateFilter<T>(Device.Get(), GetCommandQueue()->GetCommandList().Get(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat));
}
