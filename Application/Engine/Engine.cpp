
#include "Engine.h"

#include "../Application.h"
#include "../Test/Test.h"
#include "../Window/Window.h"
#include "Exception.h"
#include "Logger.h"
#include "../../Objects/Geometry/Wave/Waves.h"
#include "Textures/DDSTextureLoader/DDSTextureLoader.h"

#include <DirectXMath.h>

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
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	THROW_IF_FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&Factory)));

	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (const auto adapter = GetAdapter(false))
	{
		Device = CreateDevice(adapter);
	}
	if (Device)
	{
		DirectCommandQueue = make_shared<OCommandQueue>(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		ComputeCommandQueue = make_shared<OCommandQueue>(Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		CopyCommandQueue = make_shared<OCommandQueue>(Device, D3D12_COMMAND_LIST_TYPE_COPY);

		RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DSVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CBVSRVUAVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	else
	{
		LOG(Error, "Failed to create device");
	}

	bIsTearingSupported = CheckTearingSupport();
	CreateWindow();

	return true;
}

shared_ptr<OWindow> OEngine::GetWindow() const
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

int OEngine::Run(shared_ptr<OTest> Test)
{
	LOG(Log, "Engine::Run")

	Tests[Test->GetWindow()->GetHWND()] = Test;
	Test->Initialize();
	return 0;
}

shared_ptr<OCommandQueue> OEngine::GetCommandQueue(D3D12_COMMAND_LIST_TYPE Type)
{
	shared_ptr<OCommandQueue> queue;

	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		queue = DirectCommandQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		queue = ComputeCommandQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		queue = CopyCommandQueue;
		break;
	}
	return queue;
}

void OEngine::OnEnd(shared_ptr<OTest> Test) const
{
	FlushGPU();
	Test->UnloadContent();
	Test->Destroy();
}

void OEngine::BuildFrameResource(uint32_t PassCount)
{
	for (int i = 0; i < SRenderConstants::NumFrameResources; ++i)
	{
		FrameResources.push_back(make_unique<SFrameResource>(
			Device.Get(),
			PassCount,
			AllRenderItems.size(),
			Waves->GetVertexCount(),
			Materials.size()));
	}
}

void OEngine::OnRender(const UpdateEventArgs& Args) const
{
	LOG(Log, "Engine::OnRender")

	for (const auto val : Tests | std::views::values)
	{
		val->OnUpdate(Args);
		val->OnRender(Args);
	}
}

void OEngine::DestroyWindow()
{
	OApplication::Get()->DestroyWindow(Window);
}

void OEngine::OnWindowDestroyed()
{
}

bool OEngine::IsTearingSupported() const
{
	return bIsTearingSupported;
}

void OEngine::OnKeyPressed(KeyEventArgs& Args)
{
	// By default, do nothing.
}

void OEngine::OnKeyReleased(KeyEventArgs& Args)
{
	// By default, do nothing.
}

void OEngine::OnMouseMoved(MouseMotionEventArgs& Args)
{
	LOG(Log, "Engine::OnMouseMoved")

	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		if (const auto test = GetTestByHWND(Args.WindowHandle))
		{
			test->OnMouseMoved(Args);
		}
		window->OnMouseMoved(Args);
	}
}

void OEngine::OnMouseButtonPressed(MouseButtonEventArgs& Args)
{
	LOG(Log, "Engine::OnMouseButtonPressed")
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		if (const auto test = GetTestByHWND(Args.WindowHandle))
		{
			test->OnMouseButtonPressed(Args);
		}
		window->OnMouseButtonPressed(Args);
	}
}

void OEngine::OnMouseButtonReleased(MouseButtonEventArgs& Args)
{
	LOG(Log, "Engine::OnMouseButtonReleased")
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		if (const auto test = GetTestByHWND(Args.WindowHandle))
		{
			test->OnMouseButtonReleased(Args);
		}
		window->OnMouseButtonReleased(Args);
	}
}

void OEngine::OnMouseWheel(MouseWheelEventArgs& Args)
{
	LOG(Log, "Engine::OnMouseWheel")
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		if (const auto test = GetTestByHWND(Args.WindowHandle))
		{
			test->OnMouseWheel(Args);
		}
		window->OnMouseWheel(Args);
	}
}

void OEngine::OnResize(ResizeEventArgs& Args)
{
	LOG(Log, "Engine::OnResize")

	const auto window = GetWindowByHWND(Args.WindowHandle);
	window->OnResize(Args);
	if (const auto test = GetTestByHWND(Args.WindowHandle))
	{
		test->OnResize(Args);
	}
}

void OEngine::OnUpdateWindowSize(ResizeEventArgs& Args)
{
	const auto window = GetWindowByHWND(Args.WindowHandle);
	window->OnUpdateWindowSize(Args);
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

void OEngine::CreateWindow()
{
	Window = OApplication::Get()->CreateWindow();
	WindowsMap[Window->GetHWND()] = Window;

	Window->RegsterWindow(shared_from_this());
}

void OEngine::CheckMSAAQualitySupport()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = SRenderConstants::BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	THROW_IF_FAILED(Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	Msaa4xQuality = msQualityLevels.NumQualityLevels;
	assert(Msaa4xQuality > 0 && "Unexpected MSAA quality level.");
}

bool OEngine::GetMSAAState(UINT& Quality) const
{
	Quality = Msaa4xQuality;
	return Msaa4xState;
}

vector<SRenderItem*>& OEngine::GetOpaqueRenderItems()
{
	return RenderItems[SRenderLayer::Opaque];
}

vector<SRenderItem*>& OEngine::GetAlphaTestedRenderItems()
{
	return RenderItems[SRenderLayer::AlphaTested];
}

vector<SRenderItem*>& OEngine::GetMirrorsRenderItems()
{
	return RenderItems[SRenderLayer::Mirror];
}

vector<SRenderItem*>& OEngine::GetReflectedRenderItems()
{
	return RenderItems[SRenderLayer::Reflected];
}

void OEngine::BuildPSOs(ComPtr<ID3D12RootSignature> RootSignature, const vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout)
{
	UINT quality = 0;
	bool msaaEnable = GetMSAAState(quality);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePSO;
	ZeroMemory(&opaquePSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	opaquePSO.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
	opaquePSO.pRootSignature = RootSignature.Get();

	auto vsShader = GetShader(SShaderTypes::VSBaseShader);
	auto psShader = GetShader(SShaderTypes::PSOpaque);

	opaquePSO.VS = { reinterpret_cast<BYTE*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	opaquePSO.PS = { reinterpret_cast<BYTE*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };

	opaquePSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePSO.SampleMask = UINT_MAX;
	opaquePSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePSO.NumRenderTargets = 1;
	opaquePSO.RTVFormats[0] = SRenderConstants::BackBufferFormat;
	opaquePSO.SampleDesc.Count = msaaEnable ? 4 : 1;
	opaquePSO.SampleDesc.Quality = msaaEnable ? (quality - 1) : 0;
	opaquePSO.DSVFormat = SRenderConstants::DepthBufferFormat;
	CreatePSO(SPSOType::Opaque, opaquePSO);

	//wireframe debug
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePSO;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	CreatePSO(SPSOType::Debug, opaqueWireframePsoDesc);

	//transparent pipeline
	auto transparent = opaquePSO;
	transparent.BlendState.RenderTarget[0] = GetTransparentBlendState();
	CreatePSO(SPSOType::Transparent, transparent);

	// alpha tested pipeline
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPSO = opaquePSO;
	auto psAlphaTestedShader = GetShader(SShaderTypes::PSAlphaTested);
	alphaTestedPSO.PS = { reinterpret_cast<BYTE*>(psAlphaTestedShader->GetBufferPointer()), psAlphaTestedShader->GetBufferSize() };
	alphaTestedPSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	CreatePSO(SPSOType::AlphaTested, alphaTestedPSO);

	//
	//PSO for stencil mirrors
	//

	CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	mirrorDSS.DepthEnable = true;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = true;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;

	mirrorDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	mirrorDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC mirrorPsoDesc = opaquePSO;
	mirrorPsoDesc.BlendState = mirrorBlendState;
	mirrorPsoDesc.DepthStencilState = mirrorDSS;
	CreatePSO(SPSOType::StencilMirrors, mirrorPsoDesc);

	//
	// PSO For Reflected objects
	//

	D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
	reflectionsDSS.DepthEnable = true;
	reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsDSS.StencilEnable = true;
	reflectionsDSS.StencilReadMask = 0xff;
	reflectionsDSS.StencilWriteMask = 0xff;

	reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectionsPsoDesc = opaquePSO;
	reflectionsPsoDesc.DepthStencilState = reflectionsDSS;
	reflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;
	CreatePSO(SPSOType::StencilReflection, reflectionsPsoDesc);

	//
	// PSO for shadow objects
	//

	// We are going to draw shadows with transparency, so base it off the transparency description.
	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = transparent;
	shadowPsoDesc.DepthStencilState = shadowDSS;
	CreatePSO(SPSOType::Shadow, shadowPsoDesc);
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


vector<SRenderItem*>& OEngine::GetTransparentRenderItems()
{
	return RenderItems[SRenderLayer::Transparent];
}

vector<SRenderItem*>& OEngine::GetShadowRenderItems()
{
	return RenderItems[SRenderLayer::Shadow];
}

ComPtr<IDXGIFactory2> OEngine::GetFactory() const
{
	return Factory;
}

void OEngine::AddMaterial(string Name, unique_ptr<SMaterial>& Material)
{
	if (Materials.contains(Name))
	{
		LOG(Error, "Material with this name already exists!");
		return;
	}
	Materials[Name] = move(Material);
}

void OEngine::CreateMaterial(const string& Name, const int32_t CBIndex, const int32_t HeapIdx, const SMaterialConstants& Constants)
{
	auto mat = make_unique<SMaterial>();
	mat->Name = Name;
	mat->MaterialCBIndex = CBIndex;
	mat->DiffuseSRVHeapIndex = HeapIdx;
	mat->MaterialConsatnts = Constants;
	AddMaterial(Name, mat);
}

const OEngine::TMaterialsMap& OEngine::GetMaterials() const
{
	return Materials;
}

SMaterial* OEngine::FindMaterial(const string& Name) const
{
	if (!Materials.contains(Name))
	{
		LOG(Error, "Material not found!");
		return nullptr;
	}
	return Materials.at(Name).get();
}

STexture* OEngine::CreateTexture(string Name, wstring FileName)
{
	if (Textures.contains(Name))
	{
		LOG(Error, "Texture with this name already exists!");
		return nullptr;
	}
	auto texture = make_unique<STexture>(Name, FileName);
	texture->Name = Name;
	texture->FileName = FileName;

	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(GetDevice().Get(),
		GetCommandQueue()->GetCommandList().Get(),
		texture->FileName.c_str(),
		texture->Resource,
		texture->UploadHeap));
	Textures[Name] = move(texture);
	return Textures[Name].get();
}

STexture* OEngine::FindTexture(string Name) const
{
	if (!Textures.contains(Name))
	{
		LOG(Error, "Texture not found!");
		return nullptr;
	}
	return Textures.at(Name).get();
}

void OEngine::AddRenderItem(string Category, unique_ptr<SRenderItem> RenderItem)
{
	RenderItems[Category].push_back(RenderItem.get());
	AllRenderItems.push_back(move(RenderItem));
}

void OEngine::AddRenderItem(const vector<string>& Categories, unique_ptr<SRenderItem> RenderItem)
{
	for (auto category : Categories)
	{
		RenderItems[category].push_back(RenderItem.get());
	}
	AllRenderItems.push_back(move(RenderItem));
}

const vector<unique_ptr<SRenderItem>>& OEngine::GetAllRenderItems()
{
	return AllRenderItems;
}

void OEngine::SetPipelineState(string PSOName)
{
	GetCommandQueue()->GetCommandList()->SetPipelineState(PSOs[PSOName].Get());
}

shared_ptr<OTest> OEngine::GetTestByHWND(HWND Handler)
{
	if (Tests.size() > 0)
	{
		const auto test = Tests.find(Handler);
		if (test != Tests.end())
		{
			return test->second;
		}
		LOG(Error, "Test not found!");
		return nullptr;
	}
	return nullptr;
}

shared_ptr<OWindow> OEngine::GetWindowByHWND(HWND Handler)
{
	const auto window = WindowsMap.find(Handler);
	if (window != WindowsMap.end())
	{
		return window->second;
	}
	LOG(Error, "Window not found!");

	return nullptr;
}

void OEngine::Destroy()
{
	OApplication::Get()->DestroyWindow(Window);
}

ComPtr<ID3D12DescriptorHeap> OEngine::CreateDescriptorHeap(UINT NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type) const
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = Type;
	desc.NumDescriptors = NumDescriptors;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	THROW_IF_FAILED(Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

UINT OEngine::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE Type) const
{
	return Device->GetDescriptorHandleIncrementSize(Type);
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

std::unordered_map<string, unique_ptr<SMeshGeometry>>& OEngine::GetSceneGeometry()
{
	return SceneGeometry;
}

void OEngine::SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry)
{
	SceneGeometry[Geometry->Name] = move(Geometry);
}

SMeshGeometry* OEngine::FindSceneGeometry(const string& Name) const
{
	if (!SceneGeometry.contains(Name))
	{
		LOG(Error, "Geometry not found!");
		return nullptr;
	}
	return SceneGeometry.at(Name).get();
}

void OEngine::BuildShaders(const wstring& ShaderPath, const string& VSShaderName, const string& PSShaderName, const D3D_SHADER_MACRO* Defines)
{
	Shaders[VSShaderName] = Utils::CompileShader(ShaderPath, Defines, "VS", "vs_5_1");
	Shaders[PSShaderName] = Utils::CompileShader(ShaderPath, Defines, "PS", "ps_5_1");
}

void OEngine::BuildShader(const wstring& ShaderPath, const string& ShaderName, const string& ShaderQualifier, const string& ShaderTarget, const D3D_SHADER_MACRO* Defines)
{
	Shaders[ShaderName] = Utils::CompileShader(ShaderPath, Defines, ShaderQualifier, ShaderTarget);
}

void OEngine::BuildVSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines)
{
	Shaders[ShaderName] = Utils::CompileShader(ShaderPath, Defines, "VS", "vs_5_1");
}

void OEngine::BuildPSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines)
{
	Shaders[ShaderName] = Utils::CompileShader(ShaderPath, Defines, "PS", "ps_5_1");
}


ComPtr<ID3DBlob> OEngine::GetShader(const string& ShaderName)
{
	if (!Shaders.contains(ShaderName))
	{
		LOG(Error, "Shader not found!");
	}
	return Shaders.at(ShaderName);
}

ComPtr<ID3D12PipelineState> OEngine::GetPSO(const string& PSOName)
{
	if (!PSOs.contains(PSOName))
	{
		LOG(Error, "Shader not found!");
	}
	return PSOs.at(PSOName);
}

OWaves* OEngine::GetWaves() const
{
	return Waves.get();
}

void OEngine::CreatePSO(const string& PSOName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc)
{
	THROW_IF_FAILED(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSOs[PSOName])));
}

