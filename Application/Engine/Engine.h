#pragma once
#include "DirectX/FrameResource.h"
#include "DirectX/RenderItem/RenderItem.h"
#include "DirectX/ShaderTypes.h"
#include "Engine/RenderTarget/Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Engine/RenderTarget/Filters/Blur/BlurFilter.h"
#include "Engine/RenderTarget/Filters/SobelFilter/SobelFilter.h"
#include "Engine/RenderTarget/ShadowMap/ShadowMap.h"
#include "ExitHelper.h"
#include "GraphicsPipelineManager/GraphicsPipelineManager.h"
#include "MaterialManager/MaterialManager.h"
#include "MeshGenerator/MeshGenerator.h"
#include "RenderGraph/Graph/RenderGraph.h"
#include "RenderTarget/CubeMap/DynamicCubeMap/DynamicCubeMapTarget.h"
#include "RenderTarget/RenderTarget.h"
#include "ShaderCompiler/Compiler.h"
#include "TextureManager/TextureManager.h"
#include "UI/UIManager/UiManager.h"

#include <dxgi1_6.h>

#include <map>

class OSSAORenderTarget;
enum ERenderGroup
{
	Textures2D,
	Textures3D,
	RenderTargets,
	ShadowTextures,
	UI
};

class OEngine : public std::enable_shared_from_this<OEngine>
{
public:
	DECLARE_DELEGATE(SOnFrameResourceChanged);

	using TWindowPtr = OWindow*;
	using TWindowMap = std::map<HWND, TWindowPtr>;
	using WindowNameMap = std::map<std::wstring, TWindowPtr>;
	using TSceneGeometryMap = std::unordered_map<string, unique_ptr<SMeshGeometry>>;
	using TRenderLayer = map<string, vector<ORenderItem*>>;
	vector<unique_ptr<SFrameResource>> FrameResources;
	SFrameResource* CurrentFrameResource = nullptr;
	UINT CurrentFrameResourceIndex = 0;

	inline static std::map<HWND, TWindowPtr> WindowsMap = {};

	static void RemoveWindow(HWND Hwnd);

	static OEngine* Get()
	{
		if (Engine == nullptr)
		{
			Engine = new OEngine();
		}
		return Engine;
	}

	virtual ~OEngine();

	virtual bool Initialize();

private:
	void InitManagers();
	void PostInitialize();
	void InitUIManager();
	void InitCompiler();
	void InitPipelineManager();

public:
	void SetFogColor(DirectX::XMFLOAT4 Color);
	void SetFogStart(float Start);
	void SetFogRange(float Range);
	OWindow* GetWindow() const;
	ComPtr<ID3D12Device2> GetDevice() const;

	void FlushGPU() const;

	int InitTests(shared_ptr<class OTest> Test);
	void PostTestInit();
	void DestroyWindow();
	void OnWindowDestroyed();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, const wstring& Name, D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE) const;
	OCommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetRenderGroupStartAddress(ERenderGroup Group);

	void OnEnd(shared_ptr<OTest> Test) const;

	void TryRebuildFrameResource();
	void UpdateBoundingSphere();
	void Draw(UpdateEventArgs& Args);
	void Render(UpdateEventArgs& Args);
	void Update(UpdateEventArgs& Args);
	void OnUpdate(UpdateEventArgs& Args);
	void OnKeyPressed(KeyEventArgs& Args);
	void OnKeyReleased(KeyEventArgs& Args);
	void OnMouseMoved(class MouseMotionEventArgs& Args);
	void OnMouseButtonPressed(MouseButtonEventArgs& Args);
	void OnMouseButtonReleased(MouseButtonEventArgs& Args);
	void OnMouseWheel(MouseWheelEventArgs& Args);
	void OnResizeRequest(HWND& WindowHandle);
	void OnUpdateWindowSize(ResizeEventArgs& Args);
	void SetWindowViewport();
	bool CheckTearingSupport();
	OSSAORenderTarget* GetSSAORT() const;
	void CreateWindow();
	bool GetMSAAState(UINT& Quality) const;

	vector<ORenderItem*>& GetRenderItems(const string& Type);

	D3D12_RENDER_TARGET_BLEND_DESC GetTransparentBlendState();
	D3D12_SHADER_BYTECODE GetShaderByteCode(const string& ShaderName);
	void FillDescriptorHeaps();

	ComPtr<IDXGIFactory2> GetFactory() const;

	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;

	uint32_t GetDesiredCountOfRTVs(SResourceHeapBitFlag Flag) const;
	uint32_t GetDesiredCountOfDSVs(SResourceHeapBitFlag Flag) const;
	uint32_t GetDesiredCountOfSRVs(SResourceHeapBitFlag Flag) const;

	std::unordered_map<string, unique_ptr<SMeshGeometry>>& GetSceneGeometry();
	SMeshGeometry* SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry);
	ORenderItem* BuildRenderItemFromMesh(const string& Name, string Category, unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params);

	ORenderItem* BuildRenderItemFromMesh(string Category, unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params);
	ORenderItem* BuildRenderItemFromMesh(const string& Category, SMeshGeometry* Mesh, const SRenderItemParams& Params);
	ORenderItem* BuildRenderItemFromMesh(const string& Category, SMeshGeometry* Mesh, string Submesh, const SRenderItemParams& Params);

	OLightComponent* AddLightingComponent(ORenderItem* Item, const ELightType& Type);
	void BuildPickRenderItem();

	SMeshGeometry* FindSceneGeometry(const string& Name) const;

	ComPtr<ID3DBlob> GetShader(const string& ShaderName);

	void CreatePSO(const string& PSOName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc);
	void CreatePSO(const string& PSOName, const D3D12_COMPUTE_PIPELINE_STATE_DESC& PSODesc);
	ComPtr<ID3D12PipelineState> GetPSO(const string& PSOName);

	void AddRenderItem(string Category, unique_ptr<ORenderItem> RenderItem);
	void AddRenderItem(const vector<string>& Categories, unique_ptr<ORenderItem> RenderItem);

	const vector<unique_ptr<ORenderItem>>& GetAllRenderItems();
	void SetPipelineState(string PSOName);
	void SetPipelineState(SPSODescriptionBase* PSOInfo);

	void SetFog(DirectX::XMFLOAT4 Color, float Start, float Range);
	SPassConstants& GetMainPassCB();
	ComPtr<ID3D12DescriptorHeap>& GetDescriptorHeap();

	ID3D12RootSignature* GetDefaultRootSignature() const;
	ID3D12RootSignature* GetWavesRootSignature() const;

	vector<D3D12_INPUT_ELEMENT_DESC>& GetDefaultInputLayout();

	OBlurFilter* GetBlurFilter();
	OBilateralBlurFilter* GetBilateralBlurFilter();
	OSobelFilter* GetSobelFilter();
	void BuildFilters();

	template<typename T, typename... Args>
	T* BuildRenderObject(ERenderGroup Group, Args&&... Params);

	TUUID AddRenderObject(ERenderGroup Group, IRenderObject* RenderObject);

	void BuildOffscreenRT();
	OOffscreenTexture* GetOffscreenRT() const;
	void DrawFullScreenQuad();

	template<typename T>
	TUUID AddFilter();

	template<typename T>
	T* GetObjectByUUID(TUUID UUID, bool Checked = false);

	void SetObjectDescriptor(SRenderObjectHeap& Heap);
	void BuildDescriptorHeaps();
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescHandleForTexture(STexture* Texture) const;
	void SetAmbientLight(const DirectX::XMFLOAT4& Color);

	uint32_t GetPassCountRequired() const;

	void RebuildGeometry(string Name);
	void TryUpdateGeometry();
	void UpdateGeometryRequest(string Name);

	float GetDeltaTime() const;
	float GetTime() const;
	TRenderLayer& GetRenderLayers();

	void PerformFrustrumCulling();
	uint32_t GetTotalNumberOfInstances() const;

	OMaterialManager* GetMaterialManager() const
	{
		return MaterialManager.get();
	}

	OTextureManager* GetTextureManager() const
	{
		return TextureManager.get();
	}

	OMeshGenerator* GetMeshGenerator() const;

	void Pick(int32_t SX, int32_t SY);
	ORenderItem* GetPickedItem() const;

	SOnFrameResourceChanged OnFrameResourceChanged;
	ODynamicCubeMapRenderTarget* GetCubeRenderTarget() const;
	ODynamicCubeMapRenderTarget* BuildCubeRenderTarget(DirectX::XMFLOAT3 Center);
	void DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer);

	void UpdateMaterialCB() const;
	void UpdateLightCB(const UpdateEventArgs& Args) const;
	void UpdateObjectCB() const;
	void SetDescriptorHeap(EResourceHeapType Type);
	OShaderCompiler* GetShaderCompiler() const;
	OUIManager* GetUIManager() const;
	vector<OShadowMap*>& GetShadowMaps();
	const DirectX::BoundingSphere& GetSceneBounds() const;

	SRenderObjectHeap DefaultGlobalHeap;
	SRenderObjectHeap ShadowHeap;
	const auto& GetRenderedItems()
	{
		return RenderedItems;
	}

protected:
	void DrawRenderItemsImpl(SPSODescriptionBase* Desc, const vector<ORenderItem*>& RenderItems);
	template<typename T, typename... Args>
	T* BuildRenderObjectImpl(ERenderGroup Group, Args&&... Params);

	shared_ptr<OTest> GetTestByHWND(HWND Handler);
	OWindow* GetWindowByHWND(HWND Handler);
	void Destroy();

	OWindow* Window;
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> Adapter);

	void UpdateFrameResource();
	void InitRenderGraph();
	uint32_t GetLightComponentsCount() const;

private:
	void UpdateSSAOCB();
	void BuildSSAO();
	void RemoveRenderObject(TUUID UUID);
	void BuildFrameResource(uint32_t Count = 1);

	uint32_t PassCount = 1;
	uint32_t CurrentPass = 0;
	uint32_t CurrentNumMaterials = 0;
	uint32_t CurrentNumInstances = 0;

	OEngine() = default;
	void UpdateMainPass(const STimer& Timer);
	void GetNumLights(uint32_t& OutNumPointLights, uint32_t& OutNumSpotLights, uint32_t& OutNumDirLights) const;
	SPassConstants MainPassCB;

	ComPtr<IDXGIAdapter4> Adapter;
	ComPtr<ID3D12Device2> Device;

	unique_ptr<OCommandQueue> DirectCommandQueue;
	unique_ptr<OCommandQueue> ComputeCommandQueue;
	unique_ptr<OCommandQueue> CopyCommandQueue;

	bool bIsTearingSupported = false;

	bool Msaa4xState = false;
	UINT Msaa4xQuality = 0;

	map<HWND, shared_ptr<OTest>> Tests;
	ComPtr<IDXGIFactory4> Factory;

	TRenderLayer RenderLayers;
	vector<unique_ptr<ORenderItem>> AllRenderItems;

	std::unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	std::unordered_map<string, ComPtr<ID3D12PipelineState>> PSOs;

	TSceneGeometryMap SceneGeometry;

	ComPtr<ID3D12RootSignature> PostProcessRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> WavesRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> DefaultRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> BlurRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> BilateralBlurRootSignature = nullptr;

	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	// filters
	TUUID BlurFilterUUID;
	TUUID SobelFilterUUID;
	TUUID BilateralFilterUUID;

	bool HasInitializedTests = false;
	STimer TickTimer;
	OOffscreenTexture* OffscreenRT = nullptr;
	ODynamicCubeMapRenderTarget* CubeRenderTarget = nullptr;

	OUIManager* UIManager;

	map<ERenderGroup, vector<TUUID>> RenderGroups;
	map<TUUID, unique_ptr<IRenderObject>> RenderObjects;

	int32_t LightCount = 0;
	bool FrustrumCullingEnabled = true;

	unique_ptr<OMeshGenerator> MeshGenerator;
	unique_ptr<OTextureManager> TextureManager;
	unique_ptr<OMaterialManager> MaterialManager;

	inline static OEngine* Engine = nullptr;

	std::optional<string> GeometryToRebuild;

	ORenderItem* PickedItem = nullptr;

	unique_ptr<OShaderCompiler> ShaderCompiler;
	unique_ptr<OGraphicsPipelineManager> PipelineManager;
	unique_ptr<ORenderGraph> RenderGraph;
	vector<OLightComponent*> LightComponents;
	vector<OShadowMap*> ShadowMaps;
	DirectX::BoundingSphere SceneBounds;
	OSSAORenderTarget* SSAORT = nullptr;
	unordered_set<ORenderItem*> RenderedItems;

public:
	SDescriptorPair NullCubeSRV;
	SDescriptorPair NullTexSRV;
	ORenderGraph* GetRenderGraph() const;
};

template<typename T, typename... Args>
T* OEngine::BuildRenderObjectImpl(ERenderGroup Group, Args&&... Params)
{
	auto uuid = GenerateUUID();
	auto object = new T(std::forward<Args>(Params)...);
	object->SetID(uuid);
	object->InitRenderObject();
	RenderObjects[uuid] = unique_ptr<IRenderObject>(object);
	if (RenderGroups.contains(Group))
	{
		RenderGroups[Group].push_back(uuid);
	}
	else
	{
		RenderGroups[Group] = { uuid };
	}

	return object;
}

template<typename T, typename... Args>
T* OEngine::BuildRenderObject(ERenderGroup Group, Args&&... Params)
{
	return BuildRenderObjectImpl<T, Args...>(Group, std::forward<Args>(Params)...);
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
	return AddRenderObject(ERenderGroup::RenderTargets, OFilterBase::CreateFilter<T>(Device.Get(), GetCommandQueue(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat));
}

inline CD3DX12_GPU_DESCRIPTOR_HANDLE OEngine::GetRenderGroupStartAddress(ERenderGroup Group)
{
	if (!RenderGroups.contains(Group))
	{
		LOG(Engine, Error, "Render group not found!");
		return {};
	}
	return GetObjectByUUID<IRenderObject>(RenderGroups[Group].front())->GetSRV().GPUHandle;
}
