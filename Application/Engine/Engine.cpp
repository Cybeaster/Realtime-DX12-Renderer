
#include "Engine.h"

#include "Application.h"
#include "Camera/Camera.h"
#include "DirectX/FrameResource.h"
#include "Engine/RenderTarget/Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Engine/RenderTarget/ShadowMap/ShadowMap.h"
#include "EngineHelper.h"
#include "Exception.h"
#include "Logger.h"
#include "MathUtils.h"
#include "Profiler.h"
#include "Test/TextureTest/TextureWaves.h"
#include "TextureConstants.h"
#include "UI/Effects/FogWidget.h"
#include "UI/Effects/Light/LightWidget.h"
#include "Window/Window.h"

#include <DirectXMath.h>

#include <numeric>
#include <ranges>

using namespace Microsoft::WRL;
using namespace DirectX;

void OEngine::RemoveWindow(HWND Hwnd)
{
	const auto windowIter = WindowsMap.find(Hwnd);
	if (windowIter != WindowsMap.end())
	{
		TWindowPtr pWindow = windowIter->second;
		WindowsMap.erase(windowIter);
	}
}

OEngine::~OEngine()
{
	FlushGPU();
}

bool OEngine::Initialize()
{
	PROFILE_SCOPE()
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	THROW_IF_FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&Factory)));

	if (!DirectX::XMVerifyCPUSupport())
	{
		WIN_LOG(Default, Error, "Failed to verify DirectX Math library support.");
		return false;
	}
	if (const auto adapter = GetAdapter(false))
	{
		Device = CreateDevice(adapter);
	}
	if (Device)
	{
		DirectCommandQueue = make_unique<OCommandQueue>(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		ComputeCommandQueue = make_unique<OCommandQueue>(Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		CopyCommandQueue = make_unique<OCommandQueue>(Device, D3D12_COMMAND_LIST_TYPE_COPY);

		RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DSVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CBVSRVUAVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	else
	{
		LOG(Engine, Error, "Failed to create device");
	}

	bIsTearingSupported = CheckTearingSupport();

	InitCompiler();
	CreateWindow();
	PostInitialize();
	InitManagers();

	return true;
}

void OEngine::InitManagers()
{
	InitPipelineManager();
	InitRenderGraph();
	MeshGenerator = make_unique<OMeshGenerator>(Device.Get(), GetCommandQueue());
	TextureManager = make_unique<OTextureManager>(Device.Get(), GetCommandQueue());
	MaterialManager = make_unique<OMaterialManager>();
	MaterialManager->LoadMaterialsFromCache();
	MaterialManager->MaterialsRebuld.AddMember(this, &OEngine::TryRebuildFrameResource);
	SceneManager = make_unique<OSceneManager>();
}

void OEngine::PostInitialize()
{
	PROFILE_SCOPE()
	UIManager = BuildRenderObject<OUIManager>(UI);
	BuildFilters();
	BuildOffscreenRT();
	BuildSSAO();
	BuildNormalTangentDebugTarget();
}

void OEngine::BuildSSAO()
{
	SSAORT = BuildRenderObject<OSSAORenderTarget>(RenderTargets, Device.Get(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::NormalMapFormat);
}

void OEngine::BuildNormalTangentDebugTarget()
{
	NormalTangentDebugTarget = BuildRenderObject<ONormalTangentDebugTarget>(RenderTargets, Device.Get(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat);
}

void OEngine::BuildFilters()
{
	BlurFilterUUID = AddFilter<OGaussianBlurFilter>();
	SobelFilterUUID = AddFilter<OSobelFilter>();
	BilateralFilterUUID = AddFilter<OBilateralBlurFilter>();
}

TUUID OEngine::AddRenderObject(ERenderGroup Group, IRenderObject* RenderObject)
{
	const auto uuid = GenerateUUID();
	RenderObject->SetID(uuid);
	RenderObjects[uuid] = unique_ptr<IRenderObject>(RenderObject);
	if (RenderGroups.contains(Group)) // to do make it more generic
	{
		RenderGroups[Group].push_back(uuid);
	}
	else
	{
		RenderGroups[Group] = { uuid };
	}
	return uuid;
}

void OEngine::BuildOffscreenRT()
{
	PROFILE_SCOPE();
	OffscreenRT = BuildRenderObject<OOffscreenTexture>(RenderTargets, Device.Get(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat);
}

ODynamicCubeMapRenderTarget* OEngine::BuildCubeRenderTarget(XMFLOAT3 Center)
{
	auto resulution = SRenderConstants::CubeMapDefaultResolution;
	SRenderTargetParams cubeParams{};
	cubeParams.Width = resulution.x;
	cubeParams.Height = resulution.y;
	cubeParams.Format = SRenderConstants::BackBufferFormat;
	cubeParams.Device = Device.Get();
	cubeParams.HeapType = EResourceHeapType::Default;
	CubeRenderTarget = BuildRenderObject<ODynamicCubeMapRenderTarget>(ERenderGroup::RenderTargets, cubeParams, Center, resulution);
	return CubeRenderTarget;
}

void OEngine::DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer, bool ForceDrawAll)
{
	if (Desc == nullptr)
	{
		LOG(Engine, Warning, "PSO is nullptr!")
		return;
	}

	auto renderItems = GetRenderItems(RenderLayer);
	DrawRenderItemsImpl(Desc, renderItems, ForceDrawAll);
}

void OEngine::UpdateMaterialCB() const
{
	PROFILE_SCOPE();

	auto copyIndicesTo = [](const vector<uint32_t>& Source, uint32_t* Destination, size_t Size) {
		if (Source.size() <= Size)
		{
			std::ranges::copy(Source, Destination);
		}
	};

	unordered_set<uint32_t> updatedIndices;
	for (auto& materials = GetMaterials(); const auto& val : materials | std::views::values)
	{
		if (const auto material = val.get())
		{
			if (material->NumFramesDirty > 0)
			{
				const auto matTransform = XMLoadFloat4x4(&material->MatTransform);

				HLSL::MaterialData matConstants;
				matConstants = material->MaterialData;

				auto setTexIdx = [](HLSL::TextureData& Out, const STexturePath& Path) {
					if (Path.IsValid())
					{
						Out.TextureIndex = Path.Texture->HeapIdx;
						Out.bIsEnabled = Path.Texture->HeapIdx > 0 ? 1 : 0;
					}
				};

				setTexIdx(matConstants.AlphaMap, material->AlphaMap);
				setTexIdx(matConstants.SpecularMap, material->SpecularMap);
				setTexIdx(matConstants.AmbientMap, material->AmbientMap);
				setTexIdx(matConstants.DiffuseMap, material->DiffuseMap);
				setTexIdx(matConstants.NormalMap, material->NormalMap);
				setTexIdx(matConstants.HeightMap, material->HeightMap);

				CWIN_LOG(material->MaterialCBIndex < 0 || material->MaterialCBIndex >= TEXTURE_MAPS_NUM, Material, Error, "Material index out of bounds!")
				Put(matConstants.MatTransform, Transpose(matTransform));
				LOG(Material, Log, "Updated material: {} Index :{}", TEXT(material->Name), TEXT(material->MaterialCBIndex));
				updatedIndices.insert(material->MaterialCBIndex);
				for (auto& cb : FrameResources)
				{
					cb->MaterialBuffer->CopyData(material->MaterialCBIndex, matConstants);
					material->NumFramesDirty--;
				}
			}
		}
	}
	uint32_t currIdx = 0;
	while (updatedIndices.contains(currIdx))
	{
		updatedIndices.erase(currIdx);
		currIdx++;
	}
	LOG(Material, Log, "Updated materials till {}.", updatedIndices.size());
}

void OEngine::UpdateLightCB(const UpdateEventArgs& Args) const
{
	PROFILE_SCOPE();

	uint32_t dirIndex = 0;
	uint32_t pointIndex = 0;
	uint32_t spotIndex = 0;
	for (const auto component : LightComponents)
	{
		if (component->TryUpdate())
		{
			for (auto& cb : FrameResources)
			{
				switch (component->GetLightType())
				{
				case ELightType::Directional:
					cb->DirectionalLightBuffer->CopyData(dirIndex, component->GetDirectionalLight());
					break;
				case ELightType::Point:
					cb->PointLightBuffer->CopyData(pointIndex, component->GetPointLight());
					break;
				case ELightType::Spot:
					cb->SpotLightBuffer->CopyData(spotIndex, component->GetSpotLight());
					break;
				}
			}
			switch (component->GetLightType()) // todo make it more generic
			{
			case ELightType::Directional:
				dirIndex++;
				break;
			case ELightType::Point:
				pointIndex++;
				break;
			case ELightType::Spot:
				spotIndex++;
				break;
			}
		}
	}
}

void OEngine::DrawRenderItemsImpl(SPSODescriptionBase* Description, const vector<ORenderItem*>& RenderItems, bool bIgnoreVisibility)
{
	PROFILE_SCOPE();

	auto cmd = GetCommandQueue()->GetCommandList();
	for (size_t i = 0; i < RenderItems.size(); i++)
	{
		const auto renderItem = RenderItems[i];

		auto visibleInstances = bIgnoreVisibility ? renderItem->Instances.size() : renderItem->VisibleInstanceCount;
		if (!renderItem->IsValidChecked() || visibleInstances == 0)
		{
			continue;
		}

		PROFILE_BLOCK_START(renderItem->Name.c_str());
		if (!renderItem->Instances.empty() && renderItem->Geometry)
		{
			renderItem->BindResources(cmd.Get(), Engine->CurrentFrameResource);
			auto instanceBuffer = Engine->CurrentFrameResource->InstanceBuffer->GetResource();
			auto location = instanceBuffer->Resource->GetGPUVirtualAddress() + renderItem->StartInstanceLocation * sizeof(SInstanceData);
			GetCommandQueue()->SetResource("gInstanceData", location, Description);
			cmd->DrawIndexedInstanced(
			    renderItem->ChosenSubmesh->IndexCount,
			    visibleInstances,
			    renderItem->ChosenSubmesh->StartIndexLocation,
			    renderItem->ChosenSubmesh->BaseVertexLocation,
			    0);
		}
		PROFILE_BLOCK_END();
	}
}

OOffscreenTexture* OEngine::GetOffscreenRT() const
{
	return OffscreenRT;
}

void OEngine::DrawFullScreenQuad()
{
	const auto commandList = GetCommandQueue()->GetCommandList();
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);
}

void OEngine::InitUIManager()
{
	UIManager->InitContext(Device.Get(), Window->GetHWND(), SRenderConstants::NumFrameResources, GetDescriptorHeap().Get(), DefaultGlobalHeap, this);
}

void OEngine::InitCompiler()
{
	ShaderCompiler = make_unique<OShaderCompiler>();
	ShaderCompiler->Init();
}

void OEngine::InitPipelineManager()
{
	PipelineManager = make_unique<OGraphicsPipelineManager>();
	PipelineManager->Init();
}

void OEngine::SetFogColor(DirectX::XMFLOAT4 Color)
{
	MainPassCB.FogColor = Color;
}

void OEngine::SetFogStart(float Start)
{
	MainPassCB.FogStart = Start;
}

void OEngine::SetFogRange(float Range)
{
	MainPassCB.FogRange = Range;
}

OWindow* OEngine::GetWindow() const
{
	return Window;
}

Microsoft::WRL::ComPtr<ID3D12Device2> OEngine::GetDevice() const
{
	return Device;
}

void OEngine::FlushGPU() const
{
	DirectCommandQueue->Flush();
	ComputeCommandQueue->Flush();
	CopyCommandQueue->Flush();
}

int OEngine::InitScene()
{
	LOG(Engine, Log, "Engine::Run")

	GetCommandQueue()->TryResetCommandList();
	SceneManager->LoadScenes();
	GetCommandQueue()->ExecuteCommandListAndWait();
	PostTestInit();

	return 0;
}

void OEngine::BuildDescriptorHeaps()
{
	auto build = [&](auto& heap, EResourceHeapType Flag, wstring Name, uint32_t AddSRV = 0, uint32_t AddRTV = 0, uint32_t AddDSV = 0) {
		heap.RTVHeap = CreateDescriptorHeap(GetDesiredCountOfRTVs(Flag) + AddRTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Name + L"_RTV");
		heap.DSVHeap = CreateDescriptorHeap(GetDesiredCountOfDSVs(Flag) + AddDSV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, Name + L"_DSV");
		heap.SRVHeap = CreateDescriptorHeap(GetDesiredCountOfSRVs(Flag) + AddSRV,
		                                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		                                    Name + L"_SRV",
		                                    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		SetObjectDescriptor(heap);
	};

	build(DefaultGlobalHeap, EResourceHeapType::Default, L"GlobalHeap", UIManager->GetNumSRVRequired() + TEXTURE_MAPS_NUM + 2);
}

void OEngine::PostTestInit()
{
	PROFILE_SCOPE()
	BuildDescriptorHeaps();
	Window->InitRenderObject();
	BuildFrameResource(GetPassCountRequired());
	GetCommandQueue()->TryResetCommandList();
	FillDescriptorHeaps();
	InitUIManager();
	GetCommandQueue()->ExecuteCommandListAndWait();
	HasInitializedTests = true;
	SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	SceneBounds.Radius = 5000; // todo estimate manually
}

OCommandQueue* OEngine::GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type)
{
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return DirectCommandQueue.get();
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return ComputeCommandQueue.get();
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return CopyCommandQueue.get();
		break;
	}
	return nullptr;
}

void OEngine::OnEnd() const
{
	FlushGPU();
}

void OEngine::TryRebuildFrameResource()
{
	LOG(Engine, Log, "Engine::RebuildFrameResource")
	if (CurrentNumMaterials != MaterialManager->GetNumMaterials() || CurrentNumInstances != GetTotalNumberOfInstances())
	{
		FrameResources.clear();
		BuildFrameResource(PassCount);
	}
}

void OEngine::UpdateBoundingSphere()
{
	PROFILE_SCOPE()
	for (const auto& val : AllRenderItems)
	{
		SceneBounds.Radius = std::max(SceneBounds.Radius, std::max({ val->Bounds.Extents.x, val->Bounds.Extents.y, val->Bounds.Extents.z }));
	}
}

void OEngine::BuildFrameResource(uint32_t Count)
{
	PassCount = Count;

	CurrentNumInstances = GetTotalNumberOfInstances();
	CurrentNumMaterials = MaterialManager->GetNumMaterials();

	for (int i = 0; i < SRenderConstants::NumFrameResources; ++i)
	{
		auto frameResource = make_unique<SFrameResource>(Device.Get(), GetWindow());
		frameResource->SetPass(PassCount);
		frameResource->SetInstances(CurrentNumInstances);
		frameResource->SetMaterials(CurrentNumMaterials);
		frameResource->SetDirectionalLight(GetLightComponentsCount());
		frameResource->SetPointLight(GetLightComponentsCount());
		frameResource->SetSpotLight(GetLightComponentsCount());
		frameResource->SetSSAO();
		FrameResources.push_back(std::move(frameResource));
	}
	OnFrameResourceChanged.Broadcast();
}

void OEngine::SetDescriptorHeap(EResourceHeapType Type)
{
	switch (Type)
	{
	case EResourceHeapType::Default:
		GetCommandQueue()->SetHeap(&DefaultGlobalHeap);
		break;
	}
}

OShaderCompiler* OEngine::GetShaderCompiler() const
{
	return ShaderCompiler.get();
}

void OEngine::Draw(UpdateEventArgs& Args)
{
	PROFILE_SCOPE()
	if (HasInitializedTests)
	{
		DirectCommandQueue->TryResetCommandList();
		SetDescriptorHeap(EResourceHeapType::Default);
		Update(Args);
		Render(Args);
	}
}

void OEngine::Render(UpdateEventArgs& Args)
{
	PROFILE_SCOPE()
	Args.IsUIInfocus = UIManager->IsInFocus();

	TickTimer = Args.Timer;
	RenderGraph->Execute();
	DirectCommandQueue->ResetQueueState();
}

void OEngine::Update(UpdateEventArgs& Args)
{
	PROFILE_SCOPE()
	Args.IsUIInfocus = UIManager->IsInFocus();
	OnUpdate(Args);
	SceneManager->Update(Args);
}

void OEngine::UpdateFrameResource()
{
	PROFILE_SCOPE()
	CurrentFrameResourceIndex = (CurrentFrameResourceIndex + 1) % SRenderConstants::NumFrameResources;
	CurrentFrameResource = FrameResources[CurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame
	// resource. If not, wait until the GPU has completed commands up to
	// this fence point.
	if (CurrentFrameResource->Fence != 0 && GetCommandQueue()->GetFence()->GetCompletedValue() < CurrentFrameResource->Fence)
	{
		GetCommandQueue()->WaitForFenceValue(CurrentFrameResource->Fence);
	}
}

void OEngine::InitRenderGraph()
{
	RenderGraph = make_unique<ORenderGraph>();
	RenderGraph->Initialize(PipelineManager.get(), GetCommandQueue());
}

uint32_t OEngine::GetLightComponentsCount() const
{
	return 10; // todo fix
}

OUIManager* OEngine::GetUIManager() const
{
	return UIManager;
}

void OEngine::RemoveRenderObject(TUUID UUID)
{
	LOG(Render, Log, "Removing object with UUID: {}", TEXT(UUID));
	RenderObjects.erase(UUID);
}

void OEngine::UpdateObjectCB() const
{
	auto res = CurrentFrameResource->PassCB.get();

	int32_t idx = 1; // TODO calc frame resource automatically and calc frame resources
	for (auto& val : RenderObjects | std::views::values)
	{
		if (val->GetNumPassesRequired() == 0)
		{
			continue;
		}

		if (idx > PassCount)
		{
			LOG(Engine, Error, "Pass count exceeded!")
			break;
		}

		TUploadBufferData<SPassConstants> data;
		data.StartIndex = idx;
		data.EndIndex = idx + SCast<int32_t>(val->GetNumPassesRequired());
		data.Buffer = res;
		val->UpdatePass(data);
		idx = data.EndIndex;
	}
}

void OEngine::OnUpdate(UpdateEventArgs& Args)
{
	PROFILE_SCOPE();
	UpdateBoundingSphere();
	UpdateFrameResource();
	PerformFrustrumCulling();

	for (auto& item : AllRenderItems)
	{
		item->Update(Args);
	}

	if (CurrentFrameResource)
	{
		UpdateMainPass(Args.Timer);
		UpdateMaterialCB();
		UpdateObjectCB();
		UpdateLightCB(Args);
		UpdateSSAOCB();
	}

	for (const auto& val : RenderObjects | std::views::values)
	{
		val->Update(Args);
	}
	if (ReloadShadersRequested)
	{
		RenderGraph->ReloadShaders();
		ReloadShadersRequested = false;
	}
}

void OEngine::UpdateSSAOCB()
{
	PROFILE_SCOPE();
	SSsaoConstants constants;

	XMMATRIX T(
	    0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

	auto proj = Window->GetCamera()->GetProj();

	constants.Proj = MainPassCB.Proj;
	constants.InvProj = MainPassCB.InvProj;
	auto transposed = XMMatrixTranspose(proj * T);
	Put(constants.ProjTex, transposed);

	for (int i = 0; i < 14; i++)
	{
		constants.OffsetVectors[i] = SSAORT->GetOffsetVectors()[i];
	}

	auto blurWeights = SSAORT->CalcGaussWeights();
	constants.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
	constants.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
	constants.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);
	constants.InvRenderTargetSize = XMFLOAT2(1.0f / (SSAORT->GetWidth() / 2), 1.0f / (SSAORT->GetHeight() / 2));

	constants.OcclusionRadius = SSAORT->OcclusionRadius;
	constants.OcclusionFadeStart = SSAORT->OcclusionFadeStart;
	constants.OcclusionFadeEnd = SSAORT->OcclusionFadeEnd;
	constants.SurfaceEpsilon = SSAORT->SurfaceEpsilon;
	CurrentFrameResource->SsaoCB->CopyData(0, constants);
}

void OEngine::DestroyWindow()
{
	RemoveRenderObject(Window->GetID());
}

void OEngine::OnWindowDestroyed()
{
}

void OEngine::OnKeyPressed(KeyEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();
	if (auto window = GetWindowByHWND(Args.WindowHandle))
	{
		window->OnKeyPressed(Args);
	}

	if (UIManager)
	{
		UIManager->OnKeyboardKeyPressed(Args);
	}
}

void OEngine::OnKeyReleased(KeyEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();
	if (auto window = GetWindowByHWND(Args.WindowHandle))
	{
		window->OnKeyReleased(Args);
	}

	if (UIManager)
	{
		UIManager->OnKeyboardKeyReleased(Args);
	}
}

void OEngine::OnMouseMoved(MouseMotionEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		window->OnMouseMoved(Args);
	}
}

void OEngine::OnMouseButtonPressed(MouseButtonEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();
	UIManager->OnMouseButtonPressed(Args);
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		window->OnMouseButtonPressed(Args);
	}
}

void OEngine::OnMouseButtonReleased(MouseButtonEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();
	UIManager->OnMouseButtonReleased(Args);
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		window->OnMouseButtonReleased(Args);
	}
}

void OEngine::OnMouseWheel(MouseWheelEventArgs& Args)
{
	LOG(Engine, Log, "Engine::OnMouseWheel")
	UIManager->OnMouseWheel(Args);
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		window->OnMouseWheel(Args);
	}
}

void OEngine::OnResizeRequest(HWND& WindowHandle)
{
	PROFILE_SCOPE();
	LOG(Engine, Log, "Engine::OnResize")
	const auto window = GetWindowByHWND(WindowHandle);
	ResizeEventArgs args = { window->GetWidth(), window->GetHeight(), WindowHandle };

	window->OnResize(args);

	if (UIManager) // TODO pass all the render targets through automatically
	{
		UIManager->OnResize(args);
	}

	if (const auto blurFilter = GetBlurFilter())
	{
		blurFilter->OnResize(args.Width, args.Height);
	}

	if (const auto sobel = GetSobelFilter())
	{
		sobel->OnResize(args.Width, args.Height);
	}

	if (OffscreenRT)
	{
		OffscreenRT->OnResize(args);
	}

	if (const auto bilateralFilter = GetBilateralBlurFilter())
	{
		bilateralFilter->OnResize(args.Width, args.Height);
	}

	for (auto shadowMaps : ShadowMaps)
	{
		shadowMaps->OnResize(args);
	}

	if (SSAORT)
	{
		SSAORT->OnResize(args);
	}
}

void OEngine::OnUpdateWindowSize(ResizeEventArgs& Args)
{
	const auto window = GetWindowByHWND(Args.WindowHandle);
	window->OnUpdateWindowSize(Args);
}

void OEngine::SetWindowViewport()
{
	Window->SetViewport(GetCommandQueue()->GetCommandList().Get());
}

bool OEngine::CheckTearingSupport()
{
	BOOL allowTearing = FALSE;
	ComPtr<IDXGIFactory5> factory5;
	if (SUCCEEDED(GetFactory().As(&factory5)))
	{
		factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
		                              &allowTearing,
		                              sizeof(allowTearing));
	}

	return allowTearing == TRUE;
}

OSSAORenderTarget* OEngine::GetSSAORT() const
{
	return SSAORT;
}

void OEngine::CreateWindow()
{
	Window = OApplication::Get()->CreateWindow();
	WindowsMap[Window->GetHWND()] = Window;
	Window->RegsterWindow();
	AddRenderObject(ERenderGroup::RenderTargets, Window);
}

bool OEngine::GetMSAAState(UINT& Quality) const
{
	Quality = Msaa4xQuality;
	return Msaa4xState;
}

void OEngine::FillDescriptorHeaps()
{
	PROFILE_SCOPE();
	// Textures, SRV offset only
	uint32_t texturesOffset = 0;
	auto whiteTex = TextureManager->FindTextureByName("white1x1");
	auto buildSRV = [&, whiteTex](STexture* Texture) {
		auto pair = DefaultGlobalHeap.SRVHandle.Offset();
		Texture = Texture ? Texture : whiteTex;
		auto resourceSRV = Texture->GetSRVDesc();
		Device->CreateShaderResourceView(Texture->Resource.Resource.Get(), &resourceSRV, pair.CPUHandle);
		Texture->HeapIdx = texturesOffset;
		texturesOffset++;
	};

	auto build2DTextures = [&]() {
		for (auto& texture : TextureManager->GetTextures() | std::views::values)
		{
			if (texture->ViewType != STextureViewType::Texture2D)
			{
				continue;
			}
			buildSRV(texture.get());
		}

		while (texturesOffset < TEXTURE_MAPS_NUM)
		{
			buildSRV(nullptr);
		}
	};

	auto build3DTextures = [&]() {
		for (const auto& texture : TextureManager->GetTextures() | std::views::values) // make a separate srv heap for 3d textures
		{
			if (texture->ViewType == STextureViewType::Texture2D)
			{
				continue;
			}
			buildSRV(texture.get());
		}
	};

	build2DTextures();
	build3DTextures();

	std::for_each(RenderGroups.begin(), RenderGroups.end(), [&](const auto& objects) {
		std::for_each(objects.second.begin(), objects.second.end(), [&](const auto& id) {
			GetObjectByUUID<IRenderObject>(id)->BuildDescriptors(&DefaultGlobalHeap);
		});
	});

	NullCubeSRV = DefaultGlobalHeap.SRVHandle.Offset();
	NullTexSRV = DefaultGlobalHeap.SRVHandle.Offset();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	Device->CreateShaderResourceView(nullptr, &srvDesc, NullTexSRV.CPUHandle);
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	Device->CreateShaderResourceView(nullptr, &srvDesc, NullCubeSRV.CPUHandle);
}

D3D12_RENDER_TARGET_BLEND_DESC OEngine::GetTransparentBlendState()
{
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return transparencyBlendDesc;
}

vector<ORenderItem*>& OEngine::GetRenderItems(const string& Type)
{
	return RenderLayers[Type];
}

ComPtr<IDXGIFactory2> OEngine::GetFactory() const
{
	return Factory;
}

void OEngine::AddRenderItem(string Category, shared_ptr<ORenderItem> RenderItem)
{
	RenderLayers[Category].push_back(RenderItem.get());
	AllRenderItems.push_back(move(RenderItem));
}

void OEngine::AddRenderItem(const vector<string>& Categories, shared_ptr<ORenderItem> RenderItem)
{
	for (auto category : Categories)
	{
		RenderLayers[category].push_back(RenderItem.get());
	}
	AllRenderItems.push_back(move(RenderItem));
}

void OEngine::MoveRIToNewLayer(ORenderItem* Item, const SRenderLayer& NewLayer, const SRenderLayer& OldLayer)
{
	if (RenderLayers.contains(OldLayer))
	{
		std::erase(RenderLayers[OldLayer], Item);
	}
	RenderLayers[NewLayer].push_back(Item);
}

const vector<shared_ptr<ORenderItem>>& OEngine::GetAllRenderItems()
{
	return AllRenderItems;
}

void OEngine::SetPipelineState(string PSOName)
{
	if (PSOs.contains(PSOName) == false)
	{
		LOG(Engine, Error, "PSO not found!");
		return;
	}
	GetCommandQueue()->GetCommandList()->SetPipelineState(PSOs[PSOName].Get());
}

void OEngine::SetPipelineState(SPSODescriptionBase* PSOInfo)
{
	GetCommandQueue()->SetPipelineState(PSOInfo);
}

OGaussianBlurFilter* OEngine::GetBlurFilter()
{
	return GetObjectByUUID<OGaussianBlurFilter>(BlurFilterUUID);
}

OBilateralBlurFilter* OEngine::GetBilateralBlurFilter()
{
	return GetObjectByUUID<OBilateralBlurFilter>(BilateralFilterUUID);
}

OSobelFilter* OEngine::GetSobelFilter()
{
	return GetObjectByUUID<OSobelFilter>(SobelFilterUUID);
}

void OEngine::SetObjectDescriptor(SRenderObjectHeap& Heap)
{
	Heap.Init(CBVSRVUAVDescriptorSize, RTVDescriptorSize, DSVDescriptorSize);
}

OWindow* OEngine::GetWindowByHWND(HWND Handler)
{
	const auto window = WindowsMap.find(Handler);
	if (window != WindowsMap.end())
	{
		return window->second;
	}
	LOG(Engine, Error, "Window not found!");

	return nullptr;
}

void OEngine::Destroy()
{
	DestroyWindow();
}

ComPtr<ID3D12DescriptorHeap> OEngine::CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, const wstring& Name, D3D12_DESCRIPTOR_HEAP_FLAGS Flags) const
{
	if (NumDescriptors == 0)
	{
		return nullptr;
	}

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = Type;
	desc.NumDescriptors = NumDescriptors;
	desc.Flags = Flags;
	desc.NodeMask = 0;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	THROW_IF_FAILED(Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
	descriptorHeap->SetName(Name.c_str());
	return descriptorHeap;
}

ComPtr<IDXGIAdapter4> OEngine::GetAdapter(bool UseWarp)
{
	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (UseWarp)
	{
		THROW_IF_FAILED(Factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
		THROW_IF_FAILED(dxgiAdapter1.As(&dxgiAdapter4));
	}
	else
	{
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; Factory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually
			// creating it. The adapter with the largest dedicated video memory
			// is favored.
			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				THROW_IF_FAILED(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

ComPtr<ID3D12Device2> OEngine::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter)
{
	ComPtr<ID3D12Device2> d3d12Device2;
	THROW_IF_FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));
	//    NAME_D3D12_OBJECT(d3d12Device2);

	// Enable debug messages in debug mode.
#if defined(_DEBUG)
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		// D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE, // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		// NewFilter.DenyList.NumCategories = _countof(Categories);
		// NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		THROW_IF_FAILED(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif

	return d3d12Device2;
}

void OEngine::Pick(int32_t SX, int32_t SY)
{
	auto camera = Window->GetCamera();
	auto [origin, dir, invView] = camera->Pick(SX, SY);

	for (auto item : RenderLayers | std::views::values | std::views::join)
	{
		if (!item->bTraceable)
		{
			continue;
		}

		auto geometry = item->Geometry;
		auto world = XMLoadFloat4x4(&item->Instances[0].World);
		auto worldDet = XMMatrixDeterminant(world);
		auto Invworld = XMMatrixInverse(&worldDet, world);

		// Tranform ray to vi space of Mesh.
		auto toLocal = XMMatrixMultiply(invView, Invworld);

		origin = XMVector3TransformCoord(origin, toLocal);
		dir = XMVector3TransformNormal(dir, toLocal);

		// Make the ray direction unit length for the intersection tests.
		dir = XMVector3Normalize(dir);

		// If we hit the bounding box of the Mesh, then we might have picked a Mesh triangle,
		// so do the ray/triangle tests.
		//
		// If we did not hit the bounding box, then it is impossible that we hit
		// the Mesh, so do not waste effort doing ray/triangle tests.
		float tmin = 0.0f;
		if (item->Bounds.Intersects(origin, dir, tmin))
		{
			for (auto& submesh : geometry->GetDrawArgs() | std::views::values)
			{
				auto indices = submesh.Indices.get();
				auto vertices = submesh.Vertices.get();
				auto triCount = indices->size() / 3;

				tmin = INFINITE;
				for (size_t i = 0; i < triCount; i++)
				{
					auto i0 = indices->at(i * 3);
					auto i1 = indices->at(i * 3 + 1);
					auto i2 = indices->at(i * 3 + 2);

					auto v0 = XMLoadFloat3(&vertices->at(i0));
					auto v1 = XMLoadFloat3(&vertices->at(i1));
					auto v2 = XMLoadFloat3(&vertices->at(i2));

					if (float t = 0.0f; TriangleTests::Intersects(origin, dir, v0, v1, v2, t))
					{
						if (t < tmin)
						{
							t = tmin;

							PickedItem->bTraceable = false;
							PickedItem->Geometry = geometry;
							PickedItem->Bounds = item->Bounds;
						}
					}
				}
			}
		}
	}
}

ORenderItem* OEngine::GetPickedItem() const
{
	return PickedItem;
}

ODynamicCubeMapRenderTarget* OEngine::GetCubeRenderTarget() const
{
	return CubeRenderTarget;
}

D3D12_GPU_DESCRIPTOR_HANDLE OEngine::GetSRVDescHandleForTexture(STexture* Texture) const
{
	auto desc = DefaultGlobalHeap.SRVHeap->GetGPUDescriptorHandleForHeapStart();
	desc.ptr += Texture->HeapIdx * CBVSRVUAVDescriptorSize;
	return desc;
}

OMeshGenerator* OEngine::GetMeshGenerator() const
{
	return MeshGenerator.get();
}

void OEngine::TryUpdateGeometry()
{
	if (GeometryToRebuild.has_value())
	{
		RebuildGeometry(GeometryToRebuild.value());
		GeometryToRebuild.reset();
	}
}

void OEngine::UpdateGeometryRequest(string Name)
{
	GeometryToRebuild.emplace(Name);
}

float OEngine::GetDeltaTime() const
{
	return TickTimer.GetDeltaTime();
}

float OEngine::GetTime() const
{
	return TickTimer.GetTime();
}

OEngine::TRenderLayer& OEngine::GetRenderLayers()
{
	return RenderLayers;
}

void OEngine::PerformFrustrumCulling()
{
	PROFILE_SCOPE();
	RenderedItems.clear();
	RenderedItems.reserve(AllRenderItems.size());

	if (CurrentFrameResource == nullptr)
	{
		return;
	}

	const auto view = Window->GetCamera()->GetView();
	auto det = XMMatrixDeterminant(view);
	const auto invView = XMMatrixInverse(&det, view);
	const auto camera = Window->GetCamera();
	auto currentInstanceBuffer = CurrentFrameResource->InstanceBuffer.get();
	int32_t counter = 0;
	for (auto& e : AllRenderItems)
	{
		const auto& instData = e->Instances;
		if (e->Instances.size() == 0)
		{
			continue;
		}

		size_t visibleInstanceCount = 0;
		for (size_t i = 0; i < instData.size(); i++)
		{
			if (visibleInstanceCount == 0)
			{
				e->StartInstanceLocation = counter;
			}

			auto world = XMLoadFloat4x4(&instData[i].World);
			auto textTransform = XMLoadFloat4x4(&instData[i].TexTransform);
			det = XMMatrixDeterminant(world);
			auto invWorld = XMMatrixInverse(&det, world);

			auto viewToLocal = XMMatrixMultiply(invView, invWorld);
			BoundingFrustum localSpaceFrustum;
			camera->GetFrustrum().Transform(localSpaceFrustum, viewToLocal);

			if (localSpaceFrustum.Contains(e->Bounds) != DirectX::DISJOINT || !bFrustrumCullingEnabled)
			{
				SInstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(textTransform));
				data.MaterialIndex = instData[i].MaterialIndex;
				data.GridSpatialStep = instData[i].GridSpatialStep;
				data.DisplacementMapTexelSize = instData[i].DisplacementMapTexelSize;
				currentInstanceBuffer->CopyData(counter++, data);
				visibleInstanceCount++;
				RenderedItems.insert(e.get());
			}
		}
		e->VisibleInstanceCount = visibleInstanceCount;
	}
}

uint32_t OEngine::GetTotalNumberOfInstances() const
{
	uint32_t totalInstances = 0;
	for (const auto& e : AllRenderItems)
	{
		totalInstances += e->Instances.size();
	}
	return totalInstances;
}

void OEngine::RebuildGeometry(string Name)
{
	GetCommandQueue()->TryResetCommandList();

	auto mesh = FindSceneGeometry(Name);
	auto commandList = GetCommandQueue()->GetCommandList();

	vector<XMFLOAT3> vertices;
	vector<std::uint16_t> indices;

	for (auto& submesh : mesh->DrawArgs)
	{
		vertices.insert(vertices.end(), submesh.second.Vertices.get()->begin(), submesh.second.Vertices.get()->end());
		indices.insert(indices.end(), submesh.second.Indices.get()->begin(), submesh.second.Indices.get()->end());
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &mesh->VertexBufferCPU));
	CopyMemory(mesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &mesh->IndexBufferCPU));
	CopyMemory(mesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mesh->VertexBufferGPU = Utils::CreateDefaultBuffer(Device.Get(),
	                                                   commandList.Get(),
	                                                   vertices.data(),
	                                                   vbByteSize,
	                                                   mesh->VertexBufferUploader);

	mesh->IndexBufferGPU = Utils::CreateDefaultBuffer(Device.Get(),
	                                                  commandList.Get(),
	                                                  indices.data(),
	                                                  ibByteSize,
	                                                  mesh->IndexBufferUploader);

	GetCommandQueue()->WaitForFenceValue(GetCommandQueue()->ExecuteCommandList());
}

void OEngine::SetAmbientLight(const DirectX::XMFLOAT4& Color)
{
	MainPassCB.AmbientLight = Color;
}

uint32_t OEngine::GetPassCountRequired() const
{
	return std::accumulate(RenderObjects.begin(), RenderObjects.end(), 1, [](int acc, const auto& renderObject) {
		return acc + renderObject.second->GetNumPassesRequired();
	});
}

void OEngine::UpdateMainPass(const STimer& Timer)
{
	PROFILE_SCOPE();
	const XMMATRIX view = Window->GetCamera()->GetView();
	const XMMATRIX proj = Window->GetCamera()->GetProj();
	const XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	auto viewDet = XMMatrixDeterminant(view);
	auto projDet = XMMatrixDeterminant(proj);
	auto viewProjDet = XMMatrixDeterminant(viewProj);

	const XMMATRIX invView = XMMatrixInverse(&viewDet, view);
	const XMMATRIX invProj = XMMatrixInverse(&projDet, proj);
	const XMMATRIX invViewProj = XMMatrixInverse(&viewProjDet, viewProj);

	// clang-format off
	XMMATRIX T(
		 0.5f, 0.0f, 0.0f, 0.0f,
		 0.0f, -0.5f, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f, 0.5f, 0.0f, 1.0f);
	// clang-format on
	Put(MainPassCB.ViewProjTex, Transpose(XMMatrixMultiply(viewProj, T)));
	Put(MainPassCB.View, Transpose(view));
	Put(MainPassCB.InvView, Transpose(invView));
	Put(MainPassCB.Proj, Transpose(proj));
	Put(MainPassCB.InvProj, Transpose(invProj));
	Put(MainPassCB.ViewProj, Transpose(viewProj));
	Put(MainPassCB.InvViewProj, Transpose(invViewProj));

	MainPassCB.EyePosW = Window->GetCamera()->GetPosition3f();
	MainPassCB.RenderTargetSize = XMFLOAT2(static_cast<float>(Window->GetWidth()), static_cast<float>(Window->GetHeight()));
	MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / Window->GetWidth(), 1.0f / Window->GetHeight());
	MainPassCB.NearZ = 1.0f;
	MainPassCB.FarZ = 10000.0f;
	MainPassCB.TotalTime = Timer.GetTime();
	MainPassCB.DeltaTime = Timer.GetDeltaTime();

	GetNumLights(MainPassCB.NumPointLights, MainPassCB.NumSpotLights, MainPassCB.NumDirLights);
	const auto currPassCB = CurrentFrameResource->PassCB.get();
	currPassCB->CopyData(0, MainPassCB);
}

void OEngine::GetNumLights(uint32_t& OutNumPointLights, uint32_t& OutNumSpotLights, uint32_t& OutNumDirLights) const
{
	OutNumPointLights = 0;
	OutNumSpotLights = 0;
	OutNumDirLights = 0;
	for (auto component : LightComponents)
	{
		switch (component->GetLightType())
		{
		case ELightType::Directional:
			OutNumDirLights++;
			break;
		case ELightType::Point:
			OutNumPointLights++;
			break;
		case ELightType::Spot:
			OutNumSpotLights++;
			break;
		}
	}
}

ORenderGraph* OEngine::GetRenderGraph() const
{
	return RenderGraph.get();
}

ONormalTangentDebugTarget* OEngine::GetNormalTangentDebugTarget() const
{
	return NormalTangentDebugTarget;
}

void OEngine::ReloadShaders()
{
	ReloadShadersRequested = true;
}

uint32_t OEngine::GetDesiredCountOfSRVs(SResourceHeapBitFlag Flag) const
{
	return std::accumulate(RenderObjects.begin(), RenderObjects.end(), 0, [Flag](int32_t acc, const auto& renderObject) {
		if (renderObject.second->GetHeapType() & Flag)
		{
			return int32_t(acc + renderObject.second->GetNumSRVRequired());
		}
		return acc;
	});
}

uint32_t OEngine::GetDesiredCountOfDSVs(SResourceHeapBitFlag Flag) const
{
	return std::accumulate(RenderObjects.begin(), RenderObjects.end(), 0, [Flag](int32_t acc, const auto& renderObject) {
		if (renderObject.second->GetHeapType() & Flag)
		{
			return int32_t(acc + renderObject.second->GetNumDSVRequired());
		}
		return acc;
	});
}

uint32_t OEngine::GetDesiredCountOfRTVs(SResourceHeapBitFlag Flag) const
{
	return std::accumulate(RenderObjects.begin(), RenderObjects.end(), 0, [Flag](int32_t acc, const auto& renderObject) {
		if (renderObject.second->GetHeapType() & Flag)
		{
			return int32_t(acc + renderObject.second->GetNumRTVRequired());
		}
		return acc;
	});
}

std::unordered_map<string, unique_ptr<SMeshGeometry>>& OEngine::GetSceneGeometry()
{
	return SceneGeometry;
}

SMeshGeometry* OEngine::SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry)
{
	auto geo = Geometry.get();
	SceneGeometry[Geometry->Name] = move(Geometry);
	return geo;
}

ORenderItem* OEngine::BuildRenderItemFromMesh(unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params)
{
	return BuildRenderItemFromMesh(SetSceneGeometry(move(Mesh)), Params);
}

ORenderItem* OEngine::BuildRenderItemFromMesh(SMeshGeometry* Mesh, const SRenderItemParams& Params)
{
	if (Params.MaterialParams.Material == nullptr || Params.MaterialParams.Material->MaterialCBIndex == -1)
	{
		LOG(Geometry, Error, "Material not specified!");
	}
	ORenderItem* item = nullptr;
	if (Params.Submesh.empty())
	{
		for (const auto& key : Mesh->GetDrawArgs() | std::views::keys)
		{
			item = BuildRenderItemFromMesh(Mesh, key, Params);
		}
	}
	else
	{
		item = BuildRenderItemFromMesh(Mesh, Mesh->GetDrawArgs().begin()->first, Params);
	}
	return item;
}

ORenderItem* OEngine::BuildRenderItemFromMesh(SMeshGeometry* Mesh, const string& Submesh, const SRenderItemParams& Params)
{
	const auto submesh = Mesh->FindSubmeshGeomentry(Submesh);
	const auto mat = submesh->Material;
	auto newItem = make_shared<ORenderItem>();

	newItem->bFrustrumCoolingEnabled = Params.bFrustrumCoolingEnabled;

	uint32_t matIdx = 0;
	SRenderLayer layer;
	if (mat != nullptr)
	{
		matIdx = mat->MaterialCBIndex;
		layer = Params.OverrideLayer.value_or(mat->RenderLayer);
	}
	else
	{
		matIdx = Params.MaterialParams.Material->MaterialCBIndex;
		layer = Params.OverrideLayer.value_or(SRenderLayers::Opaque);
	}

	SInstanceData defaultInstance;
	defaultInstance.MaterialIndex = matIdx;
	CWIN_LOG(defaultInstance.MaterialIndex < 0, Geometry, Error, "Material not found!");
	defaultInstance.GridSpatialStep = Params.MaterialParams.GridSpatialStep;
	defaultInstance.DisplacementMapTexelSize = Params.MaterialParams.DisplacementMapTexelSize;
	Scale(defaultInstance.World, Params.Scale.value_or(XMFLOAT3{ 1, 1, 1 }));
	Translate(defaultInstance.World, Params.Position.value_or(XMFLOAT3{ 0, 0, 0 }));

	newItem->Instances.resize(Params.NumberOfInstances, defaultInstance);
	newItem->RenderLayer = layer;
	newItem->bIsDisplayable = Params.Displayable;
	newItem->Geometry = Mesh;
	newItem->Bounds = submesh->Bounds;
	newItem->bTraceable = Params.Pickable;
	newItem->ChosenSubmesh = Mesh->FindSubmeshGeomentry(Submesh);
	newItem->Name = Mesh->Name + "_" + Submesh + "_" + std::to_string(AllRenderItems.size());

	const auto res = newItem.get();
	if (mat)
	{
		auto weak = std::weak_ptr(newItem);
		mat->OnMaterialChanged.Add([this, weak, mat]() {
			if (weak.expired())
			{
				return;
			}
			auto res = weak.lock();
			auto oldLayer = res->RenderLayer;
			res->RenderLayer = mat->RenderLayer;
			MoveRIToNewLayer(res.get(), res->RenderLayer, oldLayer);
		});
	}
	AddRenderItem(layer, std::move(newItem));
	return res;
}

OLightComponent* OEngine::AddLightingComponent(ORenderItem* Item, const ELightType& Type)
{
	uint32_t componentNum = LightComponents.size();
	const auto res = Item->AddComponent<OLightComponent>(componentNum, componentNum, componentNum, Type);
	LightComponents.push_back(res);
	auto newShadow = BuildRenderObject<OShadowMap>(ERenderGroup::ShadowTextures, Device.Get(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), DXGI_FORMAT_R24G8_TYPELESS, res);
	ShadowMaps.push_back(newShadow);
	newShadow->SetLightIndex(componentNum);
	return res;
}

void OEngine::BuildPickRenderItem()
{
	auto newItem = make_unique<ORenderItem>();
	newItem->bFrustrumCoolingEnabled = false;
	newItem->DefaultMaterial = MaterialManager->FindMaterial(SMaterialNames::Picked);
	newItem->RenderLayer = SRenderLayers::Highlight;
	newItem->bTraceable = false;

	SInstanceData defaultInstance;
	defaultInstance.MaterialIndex = newItem->DefaultMaterial->MaterialCBIndex;

	newItem->Instances.push_back(defaultInstance);
	PickedItem = newItem.get();
	AddRenderItem(SRenderLayers::Highlight, std::move(newItem));
}

SMeshGeometry* OEngine::FindSceneGeometry(const string& Name) const
{
	if (!SceneGeometry.contains(Name))
	{
		LOG(Engine, Error, "Geometry not found!");
		return nullptr;
	}
	return SceneGeometry.at(Name).get();
}

ComPtr<ID3DBlob> OEngine::GetShader(const string& ShaderName)
{
	if (!Shaders.contains(ShaderName))
	{
		LOG(Engine, Error, "Shader not found!");
	}
	return Shaders.at(ShaderName);
}

ComPtr<ID3D12PipelineState> OEngine::GetPSO(const string& PSOName)
{
	if (!PSOs.contains(PSOName))
	{
		LOG(Engine, Error, "Shader not found!");
	}
	return PSOs.at(PSOName);
}

void OEngine::SetFog(XMFLOAT4 Color, float Start, float Range)
{
	MainPassCB.FogColor = Color;
	MainPassCB.FogStart = Start;
	MainPassCB.FogRange = Range;
}

SPassConstants& OEngine::GetMainPassCB()
{
	return MainPassCB;
}

ComPtr<ID3D12DescriptorHeap>& OEngine::GetDescriptorHeap()
{
	return DefaultGlobalHeap.SRVHeap;
}

ID3D12RootSignature* OEngine::GetDefaultRootSignature() const
{
	return DefaultRootSignature.Get();
}

ID3D12RootSignature* OEngine::GetWavesRootSignature() const
{
	return WavesRootSignature.Get();
}

vector<D3D12_INPUT_ELEMENT_DESC>& OEngine::GetDefaultInputLayout()
{
	return InputLayout;
}

void OEngine::CreatePSO(const string& PSOName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc)
{
	THROW_IF_FAILED(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSOs[PSOName])));
}

void OEngine::CreatePSO(const string& PSOName, const D3D12_COMPUTE_PIPELINE_STATE_DESC& PSODesc)
{
	THROW_IF_FAILED(Device->CreateComputePipelineState(&PSODesc, IID_PPV_ARGS(&PSOs[PSOName])));
}

ORenderItem* OEngine::BuildRenderItemFromMesh(const string& Name, string Category, unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params)
{
	auto ri = BuildRenderItemFromMesh(std::move(Mesh), Params);
	ri->Name = Name + std::to_string(AllRenderItems.size());
	return ri;
}

vector<OShadowMap*>& OEngine::GetShadowMaps()
{
	return ShadowMaps;
}

const DirectX::BoundingSphere& OEngine::GetSceneBounds() const
{
	return SceneBounds;
}
