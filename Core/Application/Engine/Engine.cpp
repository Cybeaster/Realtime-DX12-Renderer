
#include "Engine.h"

#include "Animations/AnimationManager.h"
#include "Application.h"
#include "Camera/Camera.h"
#include "DirectX/FrameResource.h"
#include "DirectX/HLSL/HlslTypes.h"
#include "Engine/RenderTarget/Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "Engine/RenderTarget/ShadowMap/ShadowMap.h"
#include "EngineHelper.h"
#include "Exception.h"
#include "GraphicsPipelineManager/GraphicsPipelineManager.h"
#include "Logger.h"
#include "MathUtils.h"
#include "MeshGenerator/MeshGenerator.h"
#include "Profiler.h"
#include "RenderGraph/Graph/RenderGraph.h"
#include "RenderTarget/Filters/Blur/BlurFilter.h"
#include "RenderTarget/Filters/SobelFilter/SobelFilter.h"
#include "TextureConstants.h"
#include "TextureManager/TextureManager.h"
#include "UI/UIManager/UiManager.h"
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

	Device = make_shared<ODevice>();
	if (!Device->Init())
	{
		WIN_LOG(Engine, Error, "Failed to create device!");
	}
	if (auto device = Device->GetDevice())
	{
		DirectCommandQueue = make_unique<OCommandQueue>(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		ComputeCommandQueue = make_unique<OCommandQueue>(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		CopyCommandQueue = make_unique<OCommandQueue>(device, D3D12_COMMAND_LIST_TYPE_COPY);

		RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DSVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CBVSRVUAVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	else
	{
		LOG(Engine, Error, "Failed to create device");
	}

	CheckRaytracingSupport();
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
	MeshGenerator = make_unique<OMeshGenerator>(Device->GetDevice(), GetCommandQueue());
	TextureManager = make_shared<OTextureManager>(Device->GetDevice(), GetCommandQueue());
	TextureManager->InitRenderObject();
	MaterialManager = make_shared<OMaterialManager>();
	MaterialManager->LoadMaterialsFromCache();
	MaterialManager->MaterialsRebuld.AddMember(this, &OEngine::TryRebuildFrameResource);
	SceneManager = make_unique<OSceneManager>();
	AnimationManager = make_unique<OAnimationManager>();
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
	SSAORT = BuildRenderObject<OSSAORenderTarget>(RenderTargets, Device, Window->GetWidth(), Window->GetHeight(), SRenderConstants::NormalMapFormat);
}

void OEngine::BuildNormalTangentDebugTarget()
{
	NormalTangentDebugTarget = BuildRenderObject<ONormalTangentDebugTarget>(RenderTargets, Device, Window->GetWidth(), Window->GetHeight(), SRenderConstants::BackBufferFormat);
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
	RenderObjects[uuid] = shared_ptr<IRenderObject>(RenderObject);
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

TUUID OEngine::AddRenderObject(ERenderGroup Group, const shared_ptr<IRenderObject>& RenderObject)
{
	auto uuid = GenerateUUID();
	RenderObject->SetID(uuid);
	RenderObjects[uuid] = RenderObject;
	if (RenderGroups.contains(Group))
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
	OffscreenRT = BuildRenderObject<OOffscreenTexture>(RenderTargets, Device, Window->GetWidth(), Window->GetHeight(), SRenderConstants::BackBufferFormat);
}

weak_ptr<ODynamicCubeMapRenderTarget> OEngine::BuildCubeRenderTarget(XMFLOAT3 Center)
{
	auto resulution = SRenderConstants::CubeMapDefaultResolution;
	SRenderTargetParams cubeParams{};
	cubeParams.Width = resulution.x;
	cubeParams.Height = resulution.y;
	cubeParams.Format = SRenderConstants::BackBufferFormat;
	cubeParams.Device = Device;
	cubeParams.HeapType = EResourceHeapType::Default;
	CubeRenderTarget = BuildRenderObject<ODynamicCubeMapRenderTarget>(ERenderGroup::RenderTargets, cubeParams, Center, resulution);

	SRenderItemParams params;
	params.Displayable = true;
	params.OverrideLayer = SRenderLayers::OpaqueDynamicReflections;
	params.SetScale({ 100, 100, 100 });
	params.Material = FindMaterial("Mirror");
	auto sphere = CreateSphereRenderItem("Sphere", params);
	return CubeRenderTarget;
}

void OEngine::DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer, bool ForceDrawAll)
{
	if (Desc == nullptr)
	{
		LOG(Engine, Warning, "PSO is nullptr!")
		return;
	}

	SDrawPayload payload;
	payload.RenderLayer = RenderLayer;
	payload.Description = Desc;
	payload.bForceDrawAll = ForceDrawAll;
	payload.InstanceBuffer = &CameraRenderedItems;
	DrawRenderItemsImpl(payload);
}

void OEngine::DrawRenderItems(SPSODescriptionBase* Desc, const string& RenderLayer, const SCulledInstancesInfo* InstanceBuffer)
{
	if (Desc == nullptr)
	{
		LOG(Engine, Warning, "PSO is nullptr!")
		return;
	}
	SDrawPayload payload(Desc, RenderLayer, InstanceBuffer, false);
	DrawRenderItemsImpl(payload);
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
						Out.TextureIndex = Path.Texture->TextureIndex;
						Out.bIsEnabled = Path.Texture->TextureIndex > 0 ? 1 : 0;
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
					cb->DirectionalLightBuffer->CopyData(dirIndex, Cast<ODirectionalLightComponent>(component)->GetDirectionalLight());
					break;
				case ELightType::Point:
					cb->PointLightBuffer->CopyData(pointIndex, Cast<OPointLightComponent>(component)->GetPointLight());
					break;
				case ELightType::Spot:
					cb->SpotLightBuffer->CopyData(spotIndex, Cast<OSpotLightComponent>(component)->GetSpotLight());
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

void OEngine::DrawRenderItemsImpl(const SDrawPayload& Payload)
{
	PROFILE_SCOPE();
	auto cmd = GetCommandQueue()->GetCommandList();
	auto graphicsPSO = Cast<SPSOGraphicsDescription>(Payload.Description);
	auto& layerRenderItems = GetRenderItems(Payload.RenderLayer);
	for (const auto& val : layerRenderItems)
	{
		if (!Payload.InstanceBuffer->Items.contains(val) && !Payload.bForceDrawAll)
		{
			continue;
		}

		auto [item, startInstanceLocation, visibleInstanceCount] = Payload.InstanceBuffer->Items.at(val);
		if (item.expired())
		{
			layerRenderItems.erase(item);
			continue;
		}

		//extract overridden geometry
		const auto renderItem = item.lock();
		const auto geometry = !Payload.OverrideGeometry.expired() ? Payload.OverrideGeometry : renderItem->Geometry;
		const auto submesh = !Payload.OverrideSubmesh.expired() ? Payload.OverrideSubmesh : renderItem->ChosenSubmesh;

		//check if frustum-culled
		auto visibleInstances = Payload.bForceDrawAll ? renderItem->Instances.size() : visibleInstanceCount;
		if (!renderItem->IsValidChecked() || visibleInstances == 0)
		{
			continue;
		}

		//draw
		PROFILE_BLOCK_START(renderItem->Name.c_str());
		if (!renderItem->Instances.empty() && !renderItem->Geometry.expired())
		{
			cmd->IASetPrimitiveTopology(graphicsPSO->PrimitiveTopologyType);
			BindMesh(geometry.lock().get());
			auto instanceBuffer = GetCurrentFrameInstBuffer(Payload.InstanceBuffer->BufferId);
			auto location = instanceBuffer->GetGPUAddress() + startInstanceLocation * sizeof(HLSL::InstanceData);
			GetCommandQueue()->SetResource(STRINGIFY_MACRO(INSTANCE_DATA), location, Payload.Description);
			cmd->DrawIndexedInstanced(
			    submesh.lock()->IndexCount,
			    visibleInstances,
			    submesh.lock()->StartIndexLocation,
			    submesh.lock()->BaseVertexLocation,
			    0);
		}
		PROFILE_BLOCK_END();
	}
}

void OEngine::DrawAABBOfRenderItems(SPSODescriptionBase* Desc)
{
	const auto commandList = GetCommandQueue()->GetCommandList();
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	auto instanceBuffer = GetCurrentFrameInstBuffer(CameraInstanceBufferID);
	auto location = instanceBuffer->GetGPUAddress();
	GetCommandQueue()->SetResource(STRINGIFY_MACRO(INSTANCE_DATA), location, Desc);
	commandList->DrawInstanced(1, CameraRenderedItems.InstanceCount, 0, 0);
}

weak_ptr<OOffscreenTexture> OEngine::GetOffscreenRT() const
{
	return OffscreenRT;
}

void OEngine::DrawFullScreenQuad(SPSODescriptionBase* Desc)
{
	const auto commandList = GetCommandQueue()->GetCommandList();
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);
};

void OEngine::DrawOnePoint()
{
	const auto commandList = GetCommandQueue()->GetCommandList();
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	commandList->DrawInstanced(1, 1, 0, 0);
}

void OEngine::DrawBox(SPSODescriptionBase* Desc)
{
	SDrawPayload payload;
	payload.Description = Desc;
	payload.RenderLayer = SRenderLayers::Opaque;
	payload.bForceDrawAll = true;
	payload.InstanceBuffer = &CameraRenderedItems;
	payload.OverrideGeometry = DebugBoxRenderItem.lock()->Geometry;

	DrawRenderItemsImpl(payload);
}

void OEngine::InitUIManager()
{
	UIManager.lock()->InitContext(Device->GetDevice(), Window->GetHWND(), SRenderConstants::NumFrameResources, GetDescriptorHeap().Get(), DefaultGlobalHeap, this);
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

void OEngine::BuildCustomGeometry()
{
	SRenderItemParams params;
	params.Material = FindMaterial("White");
	params.NumberOfInstances = 1;
	params.bFrustumCoolingEnabled = false;
	params.OverrideLayer = SRenderLayers::OneFullscreenQuad;
	params.Pickable = false;
	params.Displayable = false;
	QuadRenderItem = CreateQuadRenderItem("Quad", params);
}

void OEngine::TryCreateFrameResources()
{
	if (FrameResources.empty())
	{
		for (int i = 0; i < SRenderConstants::NumFrameResources; ++i)
		{
			auto frameResource = make_unique<SFrameResource>(Device, GetWindow());
			FrameResources.push_back(std::move(frameResource));
		}
	}
}
void OEngine::BuildDebugGeometry()
{
}

vector<OUploadBuffer<HLSL::InstanceData>*> OEngine::GetInstanceBuffersByUUID(TUUID Id) const
{
	vector<OUploadBuffer<HLSL::InstanceData>*> result;
	for (const auto& frame : FrameResources)
	{
		if (frame->InstanceBuffers.contains(Id))
		{
			result.push_back(frame->InstanceBuffers.at(Id).get());
		}
	}
	return result;
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

weak_ptr<OWindow> OEngine::GetWindow() const
{
	return Window;
}

weak_ptr<ODevice> OEngine::GetDevice() const
{
	return Device;
}

void OEngine::FlushGPU() const
{
	DirectCommandQueue->Flush();
	ComputeCommandQueue->Flush();
	CopyCommandQueue->Flush();
}

TUUID OEngine::AddInstanceBuffer(const wstring& Name)
{
	auto newid = GenerateUUID();
	TryCreateFrameResources();
	for (auto& frame : FrameResources)
	{
		frame->AddNewInstanceBuffer(Name, GetTotalNumberOfInstances(), newid);
	}
	return newid;
}

int OEngine::InitScene()
{
	LOG(Engine, Log, "Engine::Run")

	GetCommandQueue()->TryResetCommandList();
	SceneManager->LoadScenes();
	BuildCustomGeometry();
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

	build(DefaultGlobalHeap, EResourceHeapType::Default, L"GlobalHeap", UIManager.lock()->GetNumSRVRequired() + TEXTURE_MAPS_NUM + 2);
}

void OEngine::PostTestInit()
{
	PROFILE_SCOPE()
	CameraInstanceBufferID = AddInstanceBuffer(L"DefaultInstanceBuffer");
	TryCreateFrameResources();

	FillExpectedShadowMaps();
	BuildDescriptorHeaps();
	Window->InitRenderObject();
	RebuildFrameResource(GetPassCountRequired());
	GetCommandQueue()->TryResetCommandList();
	FillDescriptorHeaps();
	InitUIManager();
	GetCommandQueue()->ExecuteCommandListAndWait();
	HasInitializedTests = true;
	SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
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

void OEngine::RemoveRenderItems()
{
	PROFILE_SCOPE()
	if (PendingRemoveItems.size() >= GarbageMaxItems)
	{
		for (auto& item : PendingRemoveItems)
		{
			erase_if(AllRenderItems, [&item](const auto& val) { return val.get() == item.get(); });
			SceneGeometry.erase(item->Geometry.lock()->Name);
			LOG(Render, Log, "Removed item: {}", TEXT(item->Name)); // todo optimize
		}
		PendingRemoveItems.clear();
	}
}

void OEngine::TryRebuildFrameResource()
{
	LOG(Engine, Log, "Engine::RebuildFrameResource")
	if (CurrentNumMaterials != MaterialManager->GetNumMaterials() || CurrentNumInstances * InstanceBufferMultiplier < GetTotalNumberOfInstances())
	{
		RebuildFrameResource(PassCount);
	}
}

void OEngine::UpdateBoundingSphere()
{
	PROFILE_SCOPE()
	for (const auto& val : AllRenderItems)
	{
		SceneBounds.Radius = std::max(SceneBounds.Radius, std::max({ val->Bounds.Extents.x, val->Bounds.Extents.y, val->Bounds.Extents.z }));
	}
	Window->UpdateCameraClip(GetSceneBounds().Radius * 2);
}

void OEngine::RebuildFrameResource(uint32_t Count)
{
	PassCount = Count;

	CurrentNumInstances = GetTotalNumberOfInstances();
	CurrentNumMaterials = MaterialManager->GetNumMaterials();

	for (const auto& frame : FrameResources)
	{
		frame->SetPass(PassCount);
		frame->RebuildInstanceBuffers(CurrentNumInstances * InstanceBufferMultiplier);
		frame->SetMaterials(CurrentNumMaterials);
		frame->SetDirectionalLight(GetLightComponentsCount());
		frame->SetPointLight(GetLightComponentsCount());
		frame->SetSpotLight(GetLightComponentsCount());
		frame->SetSSAO();
		frame->SetFrusturmCorners();
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
		RemoveRenderItems();
		DirectCommandQueue->TryResetCommandList();
		SetDescriptorHeap(EResourceHeapType::Default);
		Update(Args);
		Render(Args);
	}
}

void OEngine::Render(UpdateEventArgs& Args)
{
	PROFILE_SCOPE()
	auto manager = UIManager.lock();

	Args.IsUIInfocus = manager->IsInFocus();

	TickTimer = Args.Timer;
	RenderGraph->Execute();
	DirectCommandQueue->ResetQueueState();
}

void OEngine::Update(UpdateEventArgs& Args)
{
	auto manager = UIManager.lock();
	PROFILE_SCOPE()
	Args.IsUIInfocus = manager->IsInFocus();
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

void OEngine::CheckRaytracingSupport()
{
	if (!Device)
	{
		LOG(Engine, Error, "Device is nullptr!")
		return;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
	THROW_IF_FAILED(Device->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
	if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		LOG(Engine, Error, "Raytracing not supported on this device!")
	}
}
IDXGIFactory4* OEngine::GetFactory()
{
	return Device->GetFactory();
}

void OEngine::UpdateCameraCB()
{
	for (auto& frame : FrameResources)
	{
		HLSL::CameraMatrixBuffer cb;
		auto camera = Window->GetCamera().lock();
		const auto view = camera->GetView();
		const auto proj = camera->GetProj();
		const auto viewProj = XMMatrixMultiply(view, proj);
		const auto invViewProj = Inverse(viewProj);
		Put(cb.gCamViewProj, Transpose(viewProj));
		Put(cb.gCamInvViewProj, Transpose(invViewProj));
		frame->CameraMatrixBuffer->CopyData(0, cb);
	}
}

weak_ptr<OUIManager> OEngine::GetUIManager() const
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

void OEngine::RemoveItemInstances(UpdateEventArgs& Args)
{
	for (const auto& item : AllRenderItems)
	{
		item->Update(Args);
		for (auto it = item->Instances.begin(); it != item->Instances.end();)
		{
			if (it->Lifetime.has_value())
			{
				it->Lifetime = it->Lifetime.value() - Args.Timer.GetDeltaTime();
				if (it->Lifetime.value() <= 0)
				{
					it = item->Instances.erase(it);
					if (item->Instances.empty())
					{
						PendingRemoveItems.insert(item);
					}
					continue;
				}
			}
			++it;
		}
		//LOG(Render, Log, "Removed instances from item: {}", TEXT(item->Name));
	}
}

void OEngine::OnUpdate(UpdateEventArgs& Args)
{
	PROFILE_SCOPE();

	RemoveItemInstances(Args);
	UpdateBoundingSphere();
	TryRebuildFrameResource();
	UpdateFrameResource();

	if (CurrentFrameResource)
	{
		auto camera = Window->GetCamera().lock();
		CameraRenderedItems = PerformFrustumCulling(&camera->GetFrustum(), Inverse(camera->GetView()), CameraInstanceBufferID);

		UpdateMainPass(Args.Timer);
		UpdateMaterialCB();
		UpdateObjectCB();
		UpdateLightCB(Args);
		UpdateSSAOCB();
		UpdateCameraCB();
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
	auto ssao = SSAORT.lock();
	XMMATRIX T(
	    0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

	auto proj = Window->GetCamera().lock()->GetProj();

	constants.Proj = MainPassCB.Proj;
	constants.InvProj = MainPassCB.InvProj;
	auto transposed = XMMatrixTranspose(proj * T);
	Put(constants.ProjTex, transposed);

	for (int i = 0; i < 14; i++)
	{
		constants.OffsetVectors[i] = ssao->GetOffsetVectors()[i];
	}

	auto blurWeights = ssao->CalcGaussWeights();
	constants.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
	constants.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
	constants.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);
	constants.InvRenderTargetSize = XMFLOAT2(1.0f / (ssao->GetWidth() / 2), 1.0f / (ssao->GetHeight() / 2));

	constants.OcclusionRadius = ssao->OcclusionRadius;
	constants.OcclusionFadeStart = ssao->OcclusionFadeStart;
	constants.OcclusionFadeEnd = ssao->OcclusionFadeEnd;
	constants.SurfaceEpsilon = ssao->SurfaceEpsilon;
	CurrentFrameResource->SsaoCB->CopyData(0, constants);
}

void OEngine::DestroyWindow()
{
	RemoveRenderObject(Window->GetID());
}

void OEngine::OnWindowDestroyed()
{
}

void OEngine::DrawDebugBox(DirectX::XMFLOAT3 Center, DirectX::XMFLOAT3 Extents, DirectX::XMFLOAT4 Orientation, SColor Color, float Duration)
{
	auto world = BuildWorldMatrix(Center, Extents, Orientation);

	SInstanceData data;
	data.HlslData.OverrideColor = Color.ToFloat3();
	data.HlslData.Position = Center;
	data.HlslData.Scale = Extents;
	data.HlslData.Rotation = Orientation;
	Put(data.HlslData.World, world);
	data.Lifetime = Duration;
	data.HlslData.BoundingBoxCenter = Center;
	data.HlslData.BoundingBoxExtents = Extents;
	if (DebugBoxRenderItem.expired())
	{
		SRenderItemParams params;
		params.OverrideLayer = SRenderLayers::DebugBox;
		params.bFrustumCoolingEnabled = false;
		params.Lifetime = Duration;
		DebugBoxRenderItem = CreateBoxRenderItem("Debug_Box", params);
		return;
	}
	DebugBoxRenderItem.lock()->AddInstance(data);
	LOG(Render, Log, "Added debug box instance")
}

void OEngine::DrawDebugFrustum(const DirectX::XMFLOAT4X4& InvViewProjection, const SColor Color, float Duration)
{
	auto world = BuildWorldMatrix(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), XMFLOAT4(0, 0, 0, 0));

	SInstanceData data;
	data.HlslData.OverrideColor = Color.ToFloat3();
	data.HlslData.Position = XMFLOAT3(0, 0, 0);
	data.HlslData.Scale = XMFLOAT3(1, 1, 1);
	data.HlslData.Rotation = XMFLOAT4(0, 0, 0, 0);
	Put(data.HlslData.World, world);
	data.Lifetime = Duration;
	data.HlslData.BoundingBoxCenter = XMFLOAT3(0, 0, 0);
	data.HlslData.BoundingBoxExtents = XMFLOAT3(1, 1, 1);
	data.HlslData.InvViewProjection = InvViewProjection;
	if (DebugFrustumRenderItem.expired())
	{
		SRenderItemParams params;
		params.OverrideLayer = SRenderLayers::FrustumDebug;
		params.bFrustumCoolingEnabled = false;
		params.Lifetime = Duration;
		DebugFrustumRenderItem = CreateBoxRenderItem("Debug_Frustum", params);
		return;
	}
	DebugFrustumRenderItem.lock()->AddInstance(data);
	LOG(Render, Log, "Added debug frustum instance")
}

void OEngine::OnKeyPressed(KeyEventArgs& Args)
{
	auto manager = UIManager.lock();

	Args.IsUIInfocus = manager->IsInFocus();
	auto window = GetWindowByHWND(Args.WindowHandle);
	if (!window.expired())
	{
		window.lock()->OnKeyPressed(Args);
	}

	manager->OnKeyboardKeyPressed(Args);
}

void OEngine::OnKeyReleased(KeyEventArgs& Args)
{
	auto manager = UIManager.lock();

	Args.IsUIInfocus = manager->IsInFocus();
	auto window = GetWindowByHWND(Args.WindowHandle);
	if (!window.expired())
	{
		window.lock()->OnKeyReleased(Args);
	}
	manager->OnKeyboardKeyReleased(Args);
}

void OEngine::OnMouseMoved(MouseMotionEventArgs& Args)
{
	auto manager = UIManager.lock();

	Args.IsUIInfocus = manager->IsInFocus();
	const auto window = GetWindowByHWND(Args.WindowHandle);
	if (!window.expired())
	{
		window.lock()->OnMouseMoved(Args);
	}
}

void OEngine::OnMouseButtonPressed(MouseButtonEventArgs& Args)
{
	auto manager = UIManager.lock();
	Args.IsUIInfocus = manager->IsInFocus();
	manager->OnMouseButtonPressed(Args);
	const auto window = GetWindowByHWND(Args.WindowHandle);
	if (!window.expired())
	{
		window.lock()->OnMouseButtonPressed(Args);
	}
}

void OEngine::OnMouseButtonReleased(MouseButtonEventArgs& Args)
{
	auto manager = UIManager.lock();
	Args.IsUIInfocus = manager->IsInFocus();
	manager->OnMouseButtonReleased(Args);
	const auto window = GetWindowByHWND(Args.WindowHandle);
	if (!window.expired())
	{
		window.lock()->OnMouseButtonReleased(Args);
	}
}

void OEngine::OnMouseWheel(MouseWheelEventArgs& Args)
{
	LOG(Engine, Log, "Engine::OnMouseWheel")
	UIManager.lock()->OnMouseWheel(Args);
	const auto window = GetWindowByHWND(Args.WindowHandle);
	if (!window.expired())
	{
		window.lock()->OnMouseWheel(Args);
	}
}

void OEngine::OnResizeRequest(HWND& WindowHandle)
{
	PROFILE_SCOPE();
	LOG(Engine, Log, "Engine::OnResize")
	const auto window = GetWindowByHWND(WindowHandle);
	auto lock = window.lock();
	ResizeEventArgs args = { lock->GetWidth(), lock->GetHeight(), WindowHandle };

	lock->OnResize(args);

	if (!UIManager.expired()) // TODO pass all the render targets through automatically
	{
		UIManager.lock()->OnResize(args);
	}

	if (const auto blurFilter = GetBlurFilter())
	{
		blurFilter->OnResize(args.Width, args.Height);
	}

	if (const auto sobel = GetSobelFilter())
	{
		sobel->OnResize(args.Width, args.Height);
	}

	if (!OffscreenRT.expired())
	{
		OffscreenRT.lock()->OnResize(args);
	}

	if (const auto bilateralFilter = GetBilateralBlurFilter())
	{
		bilateralFilter->OnResize(args.Width, args.Height);
	}

	for (const auto& shadowMaps : ShadowMaps)
	{
		shadowMaps.lock()->OnResize(args);
	}

	if (!SSAORT.expired())
	{
		SSAORT.lock()->OnResize(args);
	}
}

void OEngine::OnUpdateWindowSize(ResizeEventArgs& Args)
{
	const auto window = GetWindowByHWND(Args.WindowHandle);
	window.lock()->OnUpdateWindowSize(Args);
}

void OEngine::SetWindowViewport()
{
	Window->SetViewport(GetCommandQueue()->GetCommandList().Get());
}

weak_ptr<OSSAORenderTarget> OEngine::GetSSAORT() const
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

void OEngine::FillExpectedShadowMaps()
{
	PROFILE_SCOPE();
	int32_t currSize = MAX_SHADOW_MAPS - ShadowMaps.size();
	while (currSize > 0)
	{
		BuildRenderObject<OShadowMap>(ShadowTextures, Device, SRenderConstants::ShadowMapSize, DXGI_FORMAT_R24G8_TYPELESS);
		currSize--;
	}
}

OUploadBuffer<HLSL::InstanceData>* OEngine::GetCurrentFrameInstBuffer(const TUUID& Id) const
{
	if (CurrentFrameResource && CurrentFrameResource->InstanceBuffers.contains(Id))
	{
		return CurrentFrameResource->InstanceBuffers.at(Id).get();
	}
	return nullptr;
}

void OEngine::FillDescriptorHeaps()
{
	PROFILE_SCOPE();
	// Textures, SRV offset only
	uint32_t texturesCounter = 0;
	auto whiteTex = TextureManager->FindTextureByName("white1x1");
	auto buildSRV = [&, whiteTex](STexture* Texture) {
		Texture = Texture ? Texture : whiteTex;
		auto pair = DefaultGlobalHeap.SRVHandle.Offset();
		auto resourceSRV = Texture->GetSRVDesc();
		LOG(Render, Log, "Building SRV for texture: {} of type {} at srv address {} ", TEXT(Texture->Name), TEXT(Texture->Type), TEXT(pair.Index));
		Device->GetDevice()->CreateShaderResourceView(Texture->Resource.Resource.Get(), &resourceSRV, pair.CPUHandle);
		Texture->TextureIndex = texturesCounter;
		Texture->SRV = pair;
		texturesCounter++;
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
		while (texturesCounter < TEXTURE_MAPS_NUM)
		{
			buildSRV(nullptr);
		}
	};

	auto build3DTextures = [&]() {
		for (const auto& texture : TextureManager->GetTextures() | std::views::values)
		{
			if (texture->ViewType == STextureViewType::Texture2D)
			{
				continue;
			}
			buildSRV(texture.get());
		}
	};

	auto handle = DefaultGlobalHeap.SRVHandle;
	TexturesStartAddress = handle.Offset();
	build2DTextures();
	build3DTextures();

	std::ranges::for_each(RenderGroups, [&](const auto& objects) {
		if (objects.first == ERenderGroup::None)
		{
			return;
		}
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

	Device->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, NullTexSRV.CPUHandle);
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	Device->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, NullCubeSRV.CPUHandle);
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

unordered_set<weak_ptr<ORenderItem>>& OEngine::GetRenderItems(const string& Type)
{
	return RenderLayers[Type];
}

void OEngine::AddRenderItem(string Category, shared_ptr<ORenderItem> RenderItem)
{
	RenderLayers[Category].insert(RenderItem);
	AllRenderItems.insert(move(RenderItem));
}

void OEngine::AddRenderItem(const vector<string>& Categories, const shared_ptr<ORenderItem>& RenderItem)
{
	for (auto category : Categories)
	{
		RenderLayers[category].insert(RenderItem);
	}
	AllRenderItems.insert(RenderItem);
}

void OEngine::MoveRIToNewLayer(weak_ptr<ORenderItem> Item, const SRenderLayer& NewLayer, const SRenderLayer& OldLayer)
{
	if (RenderLayers.contains(OldLayer))
	{
		RenderLayers[OldLayer].erase(Item);
	}
	RenderLayers[NewLayer].insert(Item);
}

const unordered_set<shared_ptr<ORenderItem>>& OEngine::GetAllRenderItems()
{
	return AllRenderItems;
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

weak_ptr<OWindow> OEngine::GetWindowByHWND(HWND Handler)
{
	const auto window = WindowsMap.find(Handler);
	if (window != WindowsMap.end())
	{
		return window->second;
	}
	LOG(Engine, Error, "Window not found!");

	return {};
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
	THROW_IF_FAILED(Device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
	descriptorHeap->SetName(Name.c_str());
	return descriptorHeap;
}

void OEngine::Pick(int32_t SX, int32_t SY)
{
	auto camera = Window->GetCamera().lock();
	auto [origin, dir, invView] = camera->Pick(SX, SY);

	for (auto item : RenderLayers | std::views::values | std::views::join)
	{
		if (!item.lock()->bTraceable)
		{
			continue;
		}

		auto geometry = item.lock()->Geometry;
		auto world = XMLoadFloat4x4(&item.lock()->Instances[0].HlslData.World);
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
		if (item.lock()->Bounds.Intersects(origin, dir, tmin))
		{
			for (auto& submesh : geometry.lock()->GetDrawArgs() | std::views::values)
			{
				auto indices = submesh->Indices.get();
				auto vertices = submesh->Vertices.get();
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
							PickedItem->Bounds = item.lock()->Bounds;
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

weak_ptr<ODynamicCubeMapRenderTarget> OEngine::GetCubeRenderTarget() const
{
	return CubeRenderTarget;
}

D3D12_GPU_DESCRIPTOR_HANDLE OEngine::GetSRVDescHandleForTexture(STexture* Texture) const
{
	return Texture->SRV.GPUHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE OEngine::GetSRVDescHandle(int64_t Index) const
{
	auto desc = DefaultGlobalHeap.SRVHeap->GetGPUDescriptorHandleForHeapStart();
	desc.ptr += Index * CBVSRVUAVDescriptorSize;
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

SCulledInstancesInfo OEngine::PerformFrustumCulling(IBoundingGeometry* BoundingGeometry, const DirectX::XMMATRIX& ViewMatrix, const TUUID& BufferId) const
{
	PROFILE_SCOPE();

	if (CurrentFrameResource == nullptr)
	{
		LOG(Engine, Error, "CurrentFrameResource is nullptr!")
		return {};
	}

	SCulledInstancesInfo result;
	result.BufferId = BufferId;
	const auto& buffers = GetInstanceBuffersByUUID(BufferId);
	int32_t counter = 0;
	for (auto& e : AllRenderItems)
	{
		const auto& instData = e->Instances;
		if (e->Instances.size() == 0)
		{
			continue;
		}
		SCulledRenderItem item;

		size_t visibleInstanceCount = 0;
		for (size_t i = 0; i < instData.size(); i++)
		{
			if (visibleInstanceCount == 0)
			{
				if (counter >= GetCurrentFrameInstBuffer(BufferId)->MaxOffset)
				{
					LOG(Engine, Error, "Buffer size exceeded!")
					break;
				}
				item.StartInstanceLocation = counter;
			}
			const auto world = Load(instData[i].HlslData.World);
			const auto invWorld = Inverse(world);
			const auto viewToLocal = XMMatrixMultiply(invWorld, ViewMatrix);

			if (!bFrustumCullingEnabled || !e->bFrustumCoolingEnabled || BoundingGeometry->Contains(viewToLocal, e->Bounds) != DISJOINT)
			{
				auto data = e->Instances[i];
				Put(data.HlslData.World, Transpose(world));
				Put(data.HlslData.TexTransform, Transpose(Load(instData[i].HlslData.TexTransform)));

				data.HlslData.OverrideColor = instData[i].HlslData.OverrideColor;
				data.HlslData.Position = instData[i].HlslData.Position;
				data.HlslData.Scale = instData[i].HlslData.Scale;
				data.HlslData.Rotation = instData[i].HlslData.Rotation;
				data.HlslData.MaterialIndex = instData[i].HlslData.MaterialIndex;

				result.InstanceCount++;
				for (auto* buffer : buffers)
				{
					buffer->CopyData(counter, data.HlslData);
				}
				counter++;

				visibleInstanceCount++;
			}
		}
		if (visibleInstanceCount > 0)
		{
			item.VisibleInstanceCount = visibleInstanceCount;
			item.Item = e;
			result.Items[e] = item;
		}
	}
	return result;
}

//TODO fix boilerplate
SCulledInstancesInfo OEngine::PerformBoundingBoxShadowCulling(const IBoundingGeometry* BoundingGeometry, const DirectX::XMMATRIX& ViewMatrix, const TUUID& BufferId) const // fix
{
	PROFILE_SCOPE();

	if (CurrentFrameResource == nullptr)
	{
		LOG(Engine, Error, "CurrentFrameResource is nullptr!")
		return {};
	}
	auto transformBoundingBox = [](BoundingBox Box, const XMMATRIX& Matrix) {
		XMVECTOR Center = XMLoadFloat3(&Box.Center);
		Center = XMVector3Transform(Center, Matrix);
		XMStoreFloat3(&Box.Center, Center);
		return Box;
	};
	SCulledInstancesInfo result;
	result.BufferId = BufferId;
	const auto& buffers = GetInstanceBuffersByUUID(BufferId);
	int32_t counter = 0;
	for (auto& e : AllRenderItems)
	{
		const auto& instData = e->Instances;
		if (e->Instances.size() == 0)
		{
			continue;
		}
		SCulledRenderItem item;

		size_t visibleInstanceCount = 0;
		for (size_t i = 0; i < instData.size(); i++)
		{
			if (visibleInstanceCount == 0)
			{
				if (counter >= GetCurrentFrameInstBuffer(BufferId)->MaxOffset)
				{
					LOG(Engine, Error, "Buffer size exceeded!")
					break;
				}
				item.StartInstanceLocation = counter;
			}
			BoundingBox viewBoundingBox = transformBoundingBox(e->Bounds, ViewMatrix);
			const auto world = Load(instData[i].HlslData.World);
			if (!bFrustumCullingEnabled || !e->bFrustumCoolingEnabled || BoundingGeometry->Contains(viewBoundingBox) != DISJOINT)
			{
				auto data = e->Instances[i];
				Put(data.HlslData.World, Transpose(world));
				Put(data.HlslData.TexTransform, Transpose(Load(instData[i].HlslData.TexTransform)));

				data.HlslData.OverrideColor = instData[i].HlslData.OverrideColor;
				data.HlslData.Position = instData[i].HlslData.Position;
				data.HlslData.Scale = instData[i].HlslData.Scale;
				data.HlslData.Rotation = instData[i].HlslData.Rotation;
				data.HlslData.MaterialIndex = instData[i].HlslData.MaterialIndex;

				result.InstanceCount++;
				for (auto* buffer : buffers)
				{
					buffer->CopyData(counter, data.HlslData);
				}
				counter++;

				visibleInstanceCount++;
			}
		}
		if (visibleInstanceCount > 0)
		{
			item.VisibleInstanceCount = visibleInstanceCount;
			item.Item = e;
			result.Items[e] = item;
		}
	}
	return result;
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
		vertices.insert(vertices.end(), submesh.second->Vertices.get()->begin(), submesh.second->Vertices.get()->end());
		indices.insert(indices.end(), submesh.second->Indices.get()->begin(), submesh.second->Indices.get()->end());
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &mesh->VertexBufferCPU));
	CopyMemory(mesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &mesh->IndexBufferCPU));
	CopyMemory(mesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mesh->VertexBufferGPU = Utils::CreateDefaultBuffer(Device->GetDevice(),
	                                                   commandList.Get(),
	                                                   vertices.data(),
	                                                   vbByteSize,
	                                                   mesh->VertexBufferUploader);

	mesh->IndexBufferGPU = Utils::CreateDefaultBuffer(Device->GetDevice(),
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
	auto camera = Window->GetCamera().lock();
	const XMMATRIX view = camera->GetView();
	const XMMATRIX proj = camera->GetProj();
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

	MainPassCB.EyePosW = camera->GetPosition3f();
	MainPassCB.RenderTargetSize = XMFLOAT2(static_cast<float>(Window->GetWidth()), static_cast<float>(Window->GetHeight()));
	MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / Window->GetWidth(), 1.0f / Window->GetHeight());
	MainPassCB.NearZ = Window->GetCamera().lock()->GetNearZ();
	MainPassCB.FarZ = 10000.0f;
	MainPassCB.TotalTime = Timer.GetTime();
	MainPassCB.DeltaTime = Timer.GetDeltaTime();
	MainPassCB.SSAOEnabled = SSAORT.lock()->IsEnabled();
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

weak_ptr<ONormalTangentDebugTarget> OEngine::GetNormalTangentDebugTarget() const
{
	return NormalTangentDebugTarget;
}

void OEngine::ReloadShaders()
{
	ReloadShadersRequested = true;
}

OAnimationManager* OEngine::GetAnimationManager() const
{
	return AnimationManager.get();
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

const unordered_map<string, shared_ptr<SMeshGeometry>>& OEngine::GetSceneGeometry()
{
	return SceneGeometry;
}

weak_ptr<SMeshGeometry> OEngine::SetSceneGeometry(shared_ptr<SMeshGeometry> Geometry)
{
	Geometry->Name += " " + GenerateUUID();
	SceneGeometry[Geometry->Name] = Geometry;
	return Geometry;
}

weak_ptr<ORenderItem> OEngine::BuildRenderItemFromMesh(shared_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params)
{
	return BuildRenderItemFromMesh(SetSceneGeometry(Mesh), Params);
}

weak_ptr<ORenderItem> OEngine::BuildRenderItemFromMesh(weak_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params)
{
	if (Params.Material.expired() || Params.Material.lock()->MaterialCBIndex == -1)
	{
		LOG(Geometry, Warning, "Material not specified!");
	}

	weak_ptr<ORenderItem> item;
	if (Params.Submesh.empty())
	{
		for (const auto& key : Mesh.lock()->GetDrawArgs() | std::views::keys)
		{
			item = BuildRenderItemFromMesh(Mesh, key, Params);
		}
	}
	else
	{
		item = BuildRenderItemFromMesh(Mesh, Mesh.lock()->GetDrawArgs().begin()->first, Params);
	}
	LOG(Render, Log, "Built render item from mesh: {}", TEXT(Mesh.lock()->Name));
	return item;
}

weak_ptr<ORenderItem> OEngine::BuildRenderItemFromMesh(const weak_ptr<SMeshGeometry>& Mesh, const string& Submesh, const SRenderItemParams& Params)
{
	const auto submesh = Mesh.lock()->FindSubmeshGeomentry(Submesh);
	const auto mat = submesh.lock()->Material;
	auto newItem = make_shared<ORenderItem>();

	newItem->bFrustumCoolingEnabled = Params.bFrustumCoolingEnabled;

	uint32_t matIdx = 0;
	SRenderLayer layer;
	if (mat != nullptr)
	{
		matIdx = mat->MaterialCBIndex;
		layer = Params.OverrideLayer.value_or(mat->RenderLayer);
	}
	else if (!Params.Material.expired())
	{
		matIdx = Params.Material.lock()->MaterialCBIndex;
		layer = Params.OverrideLayer.value_or(SRenderLayers::Opaque);
	}
	auto submeshLock = submesh.lock();
	layer = Params.OverrideLayer.value_or(layer);
	SInstanceData defaultInstance;
	defaultInstance.HlslData.MaterialIndex = matIdx;

	defaultInstance.HlslData.Scale = Params.Scale.value_or(XMFLOAT3{ 1, 1, 1 });
	defaultInstance.HlslData.Position = Params.Position.value_or(XMFLOAT3{ 0, 0, 0 });
	defaultInstance.HlslData.Rotation = Params.Rotation.value_or(XMFLOAT4{ 0, 0, 0, 1 });

	defaultInstance.HlslData.OverrideColor = (Params.OverrideColor.value_or(SColor::White)).ToFloat3();
	defaultInstance.HlslData.BoundingBoxCenter = submeshLock->Bounds.Center;
	defaultInstance.HlslData.BoundingBoxExtents = submeshLock->Bounds.Extents;
	defaultInstance.Lifetime = Params.Lifetime;
	Put(defaultInstance.HlslData.World, BuildWorldMatrix(defaultInstance.HlslData.Position, defaultInstance.HlslData.Scale, defaultInstance.HlslData.Rotation));

	newItem->Instances.resize(Params.NumberOfInstances, defaultInstance);
	newItem->RenderLayer = layer;
	newItem->bIsDisplayable = Params.Displayable;
	newItem->Geometry = Mesh;
	newItem->Bounds = submeshLock->Bounds;
	newItem->bTraceable = Params.Pickable;
	newItem->ChosenSubmesh = Mesh.lock()->FindSubmeshGeomentry(Submesh);
	newItem->Name = Mesh.lock()->Name + "_" + Submesh + "_" + std::to_string(AllRenderItems.size());
	const auto res = newItem.get();
	if (mat)
	{
		weak_ptr weak(newItem);
		mat->OnMaterialChanged.Add([this, weak, mat]() {
			if (weak.expired())
			{
				return;
			}
			auto res = weak.lock();
			auto oldLayer = res->RenderLayer;
			res->RenderLayer = mat->RenderLayer;
			MoveRIToNewLayer(weak, res->RenderLayer, oldLayer);
		});
	}
	SceneGeometryItemDependency[Mesh].push_back(newItem);
	AddRenderItem(layer, newItem);
	return newItem;
}

weak_ptr<OShadowMap> OEngine::CreateShadowMap()
{
	uint32_t componentNum = ShadowMaps.size();
	auto newMap = BuildRenderObject<OShadowMap>(ShadowTextures, Device, SRenderConstants::ShadowMapSize, DXGI_FORMAT_R24G8_TYPELESS);
	ShadowMaps.push_back(newMap);
	newMap.lock()->SetShadowMapIndex(componentNum);
	return newMap;
}

OSpotLightComponent* OEngine::AddSpotLightComponent(ORenderItem* Item)
{
	uint32_t componentNum = LightComponents.size();
	const auto res = Item->AddComponent<OSpotLightComponent>(componentNum);
	LightComponents.push_back(res);
	res->SetShadowMap(CreateShadowMap().lock());
	return res;
}

OPointLightComponent* OEngine::AddPointLightComponent(ORenderItem* Item)
{
	uint32_t componentNum = LightComponents.size();
	const auto res = Item->AddComponent<OPointLightComponent>(componentNum);
	LightComponents.push_back(res);
	auto newShadow = BuildRenderObject<OShadowMap>(ShadowTextures, Device, SRenderConstants::ShadowMapSize, DXGI_FORMAT_R24G8_TYPELESS);
	ShadowMaps.push_back(newShadow);
	newShadow.lock()->SetShadowMapIndex(componentNum);
	return res;
}

ODirectionalLightComponent* OEngine::AddDirectionalLightComponent(ORenderItem* Item)
{
	uint32_t componentNum = LightComponents.size();
	auto light = Item->AddComponent<ODirectionalLightComponent>(componentNum);
	LightComponents.push_back(light);
	auto csm = BuildRenderObject<OCSM>(None, Device, DXGI_FORMAT_R24G8_TYPELESS);
	light->SetCSM(csm);
	return light;
}

void OEngine::BuildPickRenderItem()
{
	auto newItem = make_shared<ORenderItem>();
	newItem->bFrustumCoolingEnabled = false;
	newItem->DefaultMaterial = MaterialManager->FindMaterial(SMaterialNames::Picked);
	newItem->RenderLayer = SRenderLayers::Highlight;
	newItem->bTraceable = false;

	SInstanceData defaultInstance;
	defaultInstance.HlslData.MaterialIndex = newItem->DefaultMaterial.lock()->MaterialCBIndex;

	newItem->Instances.push_back(defaultInstance);
	PickedItem = newItem.get();
	AddRenderItem(SRenderLayers::Highlight, newItem);
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

vector<D3D12_INPUT_ELEMENT_DESC>& OEngine::GetDefaultInputLayout()
{
	return InputLayout;
}

weak_ptr<ORenderItem> OEngine::BuildRenderItemFromMesh(const string& Name, string Category, unique_ptr<SMeshGeometry> Mesh, const SRenderItemParams& Params)
{
	auto ri = BuildRenderItemFromMesh(std::move(Mesh), Params);
	ri.lock()->Name = Name + std::to_string(AllRenderItems.size());
	return ri;
}

void OEngine::BindMesh(const SMeshGeometry* Mesh)
{
	auto idxBufferView = Mesh->IndexBufferView();
	auto vertexBufferView = Mesh->VertexBufferView();

	auto list = GetCommandQueue()->GetCommandList().Get();
	list->IASetIndexBuffer(&idxBufferView);
	list->IASetVertexBuffers(0, 1, &vertexBufferView);
}

vector<weak_ptr<OShadowMap>>& OEngine::GetShadowMaps()
{
	return ShadowMaps;
}

const DirectX::BoundingSphere& OEngine::GetSceneBounds() const
{
	return SceneBounds;
}
