#pragma once
#include "Device/Device.h"
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
#include "RenderTarget/CSM/Csm.h"
#include "RenderTarget/CubeMap/DynamicCubeMap/DynamicCubeMapTarget.h"
#include "RenderTarget/NormalTangetDebugTarget/NormalTangentDebugTarget.h"
#include "RenderTarget/RenderTarget.h"
#include "Scene/SceneManager.h"
#include "ShaderCompiler/Compiler.h"
#include "TextureManager/TextureManager.h"
#include "UI/UIManager/UiManager.h"

#include <dxgi1_6.h>

#include <map>

class OSSAORenderTarget;
enum ERenderGroup
{
	None,
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
	using TRenderLayer = map<SRenderLayer, vector<ORenderItem*>>;
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

	int InitScene();
	void PostTestInit();
	void DestroyWindow();
	void OnWindowDestroyed();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, const wstring& Name, D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE) const;
	OCommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetRenderGroupStartAddress(ERenderGroup Group);

	void OnEnd() const;

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
	OSSAORenderTarget* GetSSAORT() const;
	void CreateWindow();
	bool GetMSAAState(UINT& Quality) const;
	void FillExpectedShadowMaps();

	vector<ORenderItem*>& GetRenderItems(const SRenderLayer& Type);
	D3D12_RENDER_TARGET_BLEND_DESC GetTransparentBlendState();
	void FillDescriptorHeaps();

	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;

	uint32_t GetDesiredCountOfRTVs(SResourceHeapBitFlag Flag) const;
	uint32_t GetDesiredCountOfDSVs(SResourceHeapBitFlag Flag) const;
	uint32_t GetDesiredCountOfSRVs(SResourceHeapBitFlag Flag) const;

	std::unordered_map<string, unique_ptr<SMeshGeometry>>& GetSceneGeometry();
	SMeshGeometry* SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry);
	ORenderItem* BuildRenderItemFromMesh(const string& Name, string Category, unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params);

	ORenderItem* BuildRenderItemFromMesh(unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params);
	ORenderItem* BuildRenderItemFromMesh(SMeshGeometry* Mesh, const SRenderItemParams& Params);
	ORenderItem* BuildRenderItemFromMesh(SMeshGeometry* Mesh, const string& Submesh, const SRenderItemParams& Params);

	OSpotLightComponent* AddSpotLightComponent(ORenderItem* Item);
	OPointLightComponent* AddPointLightComponent(ORenderItem* Item);
	ODirectionalLightComponent* AddDirectionalLightComponent(ORenderItem* Item);

	void BuildPickRenderItem();

	SMeshGeometry* FindSceneGeometry(const string& Name) const;

	void AddRenderItem(string Category, shared_ptr<ORenderItem> RenderItem);
	void AddRenderItem(const vector<string>& Categories, shared_ptr<ORenderItem> RenderItem);
	void MoveRIToNewLayer(ORenderItem* Item, const SRenderLayer& NewLayer, const SRenderLayer& OldLayer);
	const vector<shared_ptr<ORenderItem>>& GetAllRenderItems();
	void SetPipelineState(string PSOName);
	void SetPipelineState(SPSODescriptionBase* PSOInfo);

	void SetFog(DirectX::XMFLOAT4 Color, float Start, float Range);
	SPassConstants& GetMainPassCB();
	ComPtr<ID3D12DescriptorHeap>& GetDescriptorHeap();

	vector<D3D12_INPUT_ELEMENT_DESC>& GetDefaultInputLayout();

	OGaussianBlurFilter* GetBlurFilter();
	OBilateralBlurFilter* GetBilateralBlurFilter();
	OSobelFilter* GetSobelFilter();
	void BuildFilters();

	template<typename T, typename... Args>
	T* BuildRenderObject(ERenderGroup Group, Args&&... Params);

	TUUID AddRenderObject(ERenderGroup Group, IRenderObject* RenderObject);

	void BuildOffscreenRT();
	OOffscreenTexture* GetOffscreenRT() const;
	void DrawFullScreenQuad();
	void DrawOnePoint();
	template<typename T>
	TUUID AddFilter();

	template<typename T>
	T* GetObjectByUUID(TUUID UUID, bool Checked = false);

	void SetObjectDescriptor(SRenderObjectHeap& Heap);
	void BuildDescriptorHeaps();
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescHandleForTexture(STexture* Texture) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescHandle(int64_t Index) const;
	void SetAmbientLight(const DirectX::XMFLOAT4& Color);

	uint32_t GetPassCountRequired() const;

	void RebuildGeometry(string Name);
	void TryUpdateGeometry();
	void UpdateGeometryRequest(string Name);

	float GetDeltaTime() const;
	float GetTime() const;
	TRenderLayer& GetRenderLayers();

	SCulledInstancesInfo PerformFrustumCulling(const DirectX::BoundingFrustum& Frustum, const DirectX::XMMATRIX& ViewMatrix, TUploadBuffer<HLSL::InstanceData>& Buffer) const;
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
	void DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer, bool ForceDrawAll = false);
	void DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer, SCulledInstancesInfo& InstanceBuffer);

	void UpdateMaterialCB() const;
	void UpdateLightCB(const UpdateEventArgs& Args) const;
	void UpdateObjectCB() const;
	void SetDescriptorHeap(EResourceHeapType Type);
	OShaderCompiler* GetShaderCompiler() const;
	OUIManager* GetUIManager() const;
	vector<OShadowMap*>& GetShadowMaps();
	const DirectX::BoundingSphere& GetSceneBounds() const;

	SRenderObjectHeap DefaultGlobalHeap;

	const auto& GetRenderedItems()
	{
		return CameraRenderedItems;
	}
	IDXGIFactory4* GetFactory();

protected:
	void DrawRenderItemsImpl(SPSODescriptionBase* Description, const vector<ORenderItem*>& RenderItems, bool bIgnoreVisibility, SCulledInstancesInfo& Info);
	template<typename T, typename... Args>
	T* BuildRenderObjectImpl(ERenderGroup Group, Args&&... Params);

	OWindow* GetWindowByHWND(HWND Handler);
	void Destroy();

	OWindow* Window;
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp);
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> Adapter);

	void UpdateFrameResource();
	void InitRenderGraph();
	uint32_t GetLightComponentsCount() const;
	void CheckRaytracingSupport();

private:
	void UpdateCameraCB();
	void UpdateSSAOCB();
	void BuildSSAO();
	void BuildNormalTangentDebugTarget();
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

	unique_ptr<OCommandQueue> DirectCommandQueue;
	unique_ptr<OCommandQueue> ComputeCommandQueue;
	unique_ptr<OCommandQueue> CopyCommandQueue;
	unique_ptr<ODevice> Device;

	bool bIsTearingSupported = false;

	bool Msaa4xState = false;
	UINT Msaa4xQuality = 0;

	TRenderLayer RenderLayers;
	vector<shared_ptr<ORenderItem>> AllRenderItems;

	TSceneGeometryMap SceneGeometry;

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
	ONormalTangentDebugTarget* NormalTangentDebugTarget = nullptr;
	SCulledInstancesInfo CameraRenderedItems;
	unique_ptr<OSceneManager> SceneManager;
	ORenderItem* QuadItem = nullptr;

public:
	bool bFrustrumCullingEnabled = true;
	bool ReloadShadersRequested = false;
	SDescriptorPair NullCubeSRV;
	SDescriptorPair NullTexSRV;
	ORenderGraph* GetRenderGraph() const;
	ONormalTangentDebugTarget* GetNormalTangentDebugTarget() const;
	void ReloadShaders();
	OShadowMap* CreateShadowMap();
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
	return AddRenderObject(ERenderGroup::RenderTargets, OFilterBase::CreateFilter<T>(Device->GetDevice(), GetCommandQueue(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat));
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
