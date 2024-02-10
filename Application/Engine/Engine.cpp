
#include "Engine.h"

#include "../Application.h"
#include "../Test/Test.h"
#include "../Window/Window.h"
#include "Camera/Camera.h"
#include "Exception.h"
#include "Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Logger.h"
#include "Settings.h"
#include "Test/TextureTest/TextureWaves.h"
#include "Textures/DDSTextureLoader/DDSTextureLoader.h"
#include "UI/Effects/FogWidget.h"
#include "UI/Effects/Light/LightWidget.h"
#include "UI/Filters/FilterManager.h"
#include "UI/Filters/GaussianBlurWidget.h"
#include "UI/Filters/SobelFilterWidget.h"
#include "UI/Geometry/GeometryManager.h"

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
		LOG(Engine, Error, "Failed to create device");
	}

	bIsTearingSupported = CheckTearingSupport();
	MeshGenerator = make_unique<OMeshGenerator>();
	CreateWindow();
	PostInitialize();
	return true;
}

void OEngine::PostInitialize()
{
	UIManager = make_unique<OUIManager>();

	BuildFilters();
	BuildOffscreenRT();
	BuildShadersAndInputLayouts();
	BuildDefaultRootSignature();
	BuildPostProcessRootSignature();
	BuildWavesRootSignature();
	BuildBlurRootSignature();
	BuildBilateralBlurRootSignature();
	BuildPSOs();
}

void OEngine::BuildFilters()
{
	BlurFilterUUID = AddFilter<OBlurFilter>();
	SobelFilterUUID = AddFilter<OSobelFilter>();
	BilateralFilterUUID = AddFilter<OBilateralBlurFilter>();
}

TUUID OEngine::AddRenderObject(IRenderObject* RenderObject)
{
	const auto uuid = GenerateUUID();
	RenderObjects[uuid] = unique_ptr<IRenderObject>(RenderObject);
	return uuid;
}

void OEngine::BuildOffscreenRT()
{
	const auto RT = new ORenderTarget(Device.Get(), GetWindow()->GetWidth(), GetWindow()->GetHeight(), SRenderConstants::BackBufferFormat);
	AddRenderObject(RT);
	OffscreenRT = RT;
}

ORenderTarget* OEngine::GetOffscreenRT() const
{
	return OffscreenRT;
}

void OEngine::DrawFullScreenQuad()
{
	const auto commandList = GetCommandQueue()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, nullptr);
	commandList->IASetIndexBuffer(nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->DrawInstanced(6, 1, 0, 0);
}

uint32_t OEngine::GetTextureNum()
{
	return static_cast<uint32_t>(Textures.size());
}

void OEngine::InitUIManager()
{
	UIManager->InitContext(Device.Get(), Window->GetHWND(), SRenderConstants::NumFrameResources, GetSRVHeap().Get(), SRVDescriptor, this);
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

int OEngine::InitTests(shared_ptr<OTest> Test)
{
	LOG(Engine, Log, "Engine::Run")

	Tests[Test->GetWindow()->GetHWND()] = Test;
	Test->Initialize();
	PostTestInit();
	return 0;
}

void OEngine::PostTestInit()
{
	BuildFrameResource(1);
	BuildDescriptorHeap();
	InitUIManager();

	HasInitializedTests = true;
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
		    GetTotalNumberOfInstances(),
		    Materials.size()));
	}
}

void OEngine::OnPreRender()
{
	if (SRVDescriptorHeap)
	{
		const auto commandList = GetCommandQueue()->GetCommandList();

		GetCommandQueue()->ResetCommandList();

		ID3D12DescriptorHeap* heaps[] = { SRVDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(heaps), heaps);

		const auto window = GetWindow();
		commandList->RSSetViewports(1, &window->Viewport);
		commandList->RSSetScissorRects(1, &window->ScissorRect);

		const auto dsv = window->GetDepthStensilView();
		const auto rtv = window->CurrentBackBufferView();

		Utils::ResourceBarrier(commandList.Get(), window->GetCurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Clear the back buffer and depth buffer.
		commandList->ClearRenderTargetView(rtv, reinterpret_cast<float*>(&MainPassCB.FogColor), 0, nullptr);
		commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to.
		commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

		commandList->SetGraphicsRootSignature(DefaultRootSignature.Get());
	}
}

void OEngine::Draw(UpdateEventArgs& Args)
{
	if (HasInitializedTests)
	{
		Update(Args);
		Render(Args);
	}
}

void OEngine::Render(UpdateEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();

	TickTimer = Args.Timer;
	OnPreRender();
	for (const auto val : Tests | std::views::values)
	{
		val->OnRender(Args);
	}
	OnPostRender();
}

void OEngine::Update(UpdateEventArgs& Args)
{
	Args.IsUIInfocus = UIManager->IsInFocus();
	OnUpdate(Args);
	for (const auto val : Tests | std::views::values)
	{
		val->OnUpdate(Args);
	}
}

void OEngine::OnUpdate(UpdateEventArgs& Args)
{
	GetWindow()->OnUpdate(Args);
	PerformFrustrumCulling();
	if (CurrentFrameResources)
	{
		UpdateMainPass(Args.Timer);
	}

	if (UIManager)
	{
		UIManager->Update();
	}
}

void OEngine::PostProcess(HWND Handler)
{
	const auto commandList = GetCommandQueue()->GetCommandList();
	const auto backBuffer = GetWindowByHWND(Handler)->GetCurrentBackBuffer().Get();

	/*Utils::ResourceBarrier(commandList.Get(),
	                       OffscreenRT->GetResource(),
	                       D3D12_RESOURCE_STATE_RENDER_TARGET,
	                       D3D12_RESOURCE_STATE_GENERIC_READ);

	Utils::ResourceBarrier(commandList.Get(),
	                       backBuffer,
	                       D3D12_RESOURCE_STATE_PRESENT,
	                       D3D12_RESOURCE_STATE_RENDER_TARGET);*/

	auto rtv = Window->CurrentBackBufferView();
	auto dsv = Window->GetDepthStensilView();
	commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

	/*if (auto [executed, srv] = GetSobelFilter()->Execute(PostProcessRootSignature.Get(),
	                                                     PSOs[SPSOType::SobelFilter].Get(),
	                                                     OffscreenRT->GetSRV());
	    executed)
	{
		DrawCompositeShader(srv);
	}
	else*/
	{
		/*Utils::ResourceBarrier(commandList.Get(),
		                       backBuffer,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET,
		                       D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->CopyResource(backBuffer, OffscreenRT->GetResource());
		Utils::ResourceBarrier(commandList.Get(),
		                       backBuffer,
		                       D3D12_RESOURCE_STATE_COPY_DEST,
		                       D3D12_RESOURCE_STATE_RENDER_TARGET);*/
	}

	/*GetBlurFilter()->Execute(BlurRootSignature.Get(),
	                         PSOs[SPSOType::HorizontalBlur].Get(),
	                         PSOs[SPSOType::VerticalBlur].Get(),
	                         backBuffer);

	GetBlurFilter()->OutputTo(backBuffer);

	GetBilateralBlurFilter()->Execute(BilateralBlurRootSignature.Get(),
	                                  PSOs[SPSOType::BilateralBlur].Get(),
	                                  backBuffer);

	GetBilateralBlurFilter()->OutputTo(backBuffer);*/

	Utils::ResourceBarrier(commandList.Get(),
	                       backBuffer,
	                       D3D12_RESOURCE_STATE_COPY_DEST,
	                       D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void OEngine::DrawCompositeShader(CD3DX12_GPU_DESCRIPTOR_HANDLE Input)

{
	const auto commandList = GetCommandQueue()->GetCommandList();
	commandList->SetGraphicsRootSignature(PostProcessRootSignature.Get());
	SetPipelineState(SPSOType::Composite);
	commandList->SetGraphicsRootDescriptorTable(0, OffscreenRT->GetSRV());
	commandList->SetGraphicsRootDescriptorTable(1, Input);
	DrawFullScreenQuad();
}

void OEngine::OnPostRender()
{
	PostProcess(Window->GetHWND());

	UIManager->Draw();
	UIManager->PostRender(GetCommandQueue()->GetCommandList().Get());

	Utils::ResourceBarrier(GetCommandQueue()->GetCommandList().Get(),
	                       Window->GetCurrentBackBuffer().Get(),
	                       D3D12_RESOURCE_STATE_RENDER_TARGET,
	                       D3D12_RESOURCE_STATE_PRESENT);

	GetCommandQueue()->ExecuteCommandList();

	THROW_IF_FAILED(Window->GetSwapChain()->Present(0, 0));
	Window->MoveToNextFrame();

	// Add an instruction to the command queue to set a new fence point.
	// Because we are on the GPU timeline, the new fence point won’t be
	// set until the GPU finishes processing all the commands prior to
	// this Signal().
	CurrentFrameResources->Fence = GetCommandQueue()->Signal();

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushGPU();
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
	UIManager->OnMouseButtonPressed(Args);
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
	UIManager->OnMouseButtonReleased(Args);
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
	LOG(Engine, Log, "Engine::OnMouseWheel")
	UIManager->OnMouseWheel(Args);
	if (const auto window = GetWindowByHWND(Args.WindowHandle))
	{
		if (const auto test = GetTestByHWND(Args.WindowHandle))
		{
			test->OnMouseWheel(Args);
		}
		window->OnMouseWheel(Args);
	}
}

void OEngine::OnResizeRequest(HWND& WindowHandle)
{
	LOG(Engine, Log, "Engine::OnResize")
	const auto window = GetWindowByHWND(WindowHandle);
	ResizeEventArgs args = { window->GetWidth(), window->GetHeight(), WindowHandle };

	window->OnResize(args);

	if (const auto test = GetTestByHWND(WindowHandle))
	{
		test->OnResize(args);
	}

	if (UIManager)
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
		OffscreenRT->OnResize(args.Width, args.Height);
	}

	if (const auto bilateralFilter = GetBilateralBlurFilter())
	{
		bilateralFilter->OnResize(args.Width, args.Height);
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

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetOpaquePSODesc()
{
	UINT quality = 0;
	bool msaaEnable = GetMSAAState(quality);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePSO;
	ZeroMemory(&opaquePSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	opaquePSO.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
	opaquePSO.pRootSignature = DefaultRootSignature.Get();

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
	return opaquePSO;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetAlphaTestedPSODesc()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPSO = GetOpaquePSODesc();
	const auto psAlphaTestedShader = GetShader(SShaderTypes::PSAlphaTested);
	alphaTestedPSO.PS = { reinterpret_cast<BYTE*>(psAlphaTestedShader->GetBufferPointer()), psAlphaTestedShader->GetBufferSize() };
	alphaTestedPSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	return alphaTestedPSO;
}
D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetTransparentPSODesc()
{
	auto transparent = GetOpaquePSODesc();
	transparent.BlendState.RenderTarget[0] = GetTransparentBlendState();
	return transparent;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetShadowPSODesc()
{
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = GetTransparentPSODesc();
	shadowPsoDesc.DepthStencilState = shadowDSS;
	return shadowPsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetReflectedPSODesc()
{
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectionsPsoDesc = GetOpaquePSODesc();
	reflectionsPsoDesc.DepthStencilState = reflectionsDSS;
	reflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;
	return reflectionsPsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetMirrorPSODesc()
{
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC mirrorPsoDesc = GetOpaquePSODesc();
	mirrorPsoDesc.BlendState = mirrorBlendState;
	mirrorPsoDesc.DepthStencilState = mirrorDSS;
	return mirrorPsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetDebugPSODesc()
{
	//wireframe debug
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = GetOpaquePSODesc();
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	return opaqueWireframePsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetCompositePSODesc()
{
	auto compositePSO = GetOpaquePSODesc();
	compositePSO.pRootSignature = PostProcessRootSignature.Get();
	compositePSO.DepthStencilState.DepthEnable = false;
	compositePSO.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	compositePSO.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	compositePSO.VS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::VSComposite)->GetBufferPointer()),
		GetShader(SShaderTypes::VSComposite)->GetBufferSize()
	};

	compositePSO.PS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::PSComposite)->GetBufferPointer()),
		GetShader(SShaderTypes::PSComposite)->GetBufferSize()
	};

	return compositePSO;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OEngine::GetSobelPSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC sobelPSO = {};
	sobelPSO.pRootSignature = PostProcessRootSignature.Get();
	sobelPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSSobelFilter)->GetBufferPointer()),
		GetShader(SShaderTypes::CSSobelFilter)->GetBufferSize()
	};
	sobelPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return sobelPSO;
}

void OEngine::BuildDescriptorHeap()
{
	auto texturesNum = GetTextureNum();
	auto renderObjectDescCount = std::accumulate(
	    std::views::values(RenderObjects).begin(),
	    std::views::values(RenderObjects).end(),
	    0,
	    [](int acc, const auto& renderObject) {
		    return acc + renderObject->GetNumDescriptors();
	    });

	auto totalDescriptors = texturesNum + renderObjectDescCount + GetNumOffscrenRT() + UIManager->GetNumDescriptors();
	auto numBackBuffers = OWindow::BuffersCount;

	// create srv heap counting all the textures in it
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = totalDescriptors;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&SRVDescriptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	//Textures, SRV offset only
	for (const auto& texture : Textures | std::views::values)
	{
		srvDesc.Format = texture->Resource->GetDesc().Format;
		Device->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, hDescriptor);
		hDescriptor.Offset(1, CBVSRVUAVDescriptorSize);
	}

	//All the other objcts, SRVs, UAVs and RTVs
	SRVDescriptor = GetObjectDescriptor();
	SRVDescriptor.OffsetSRV(texturesNum);
	SRVDescriptor.RTVCPUOffset(numBackBuffers);

	for (const auto& rObject : RenderObjects | std::views::values)
	{
		rObject->BuildDescriptors(&SRVDescriptor);
		rObject->UpdateDescriptors(SRVDescriptor);
	}
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OEngine::GetWavesRenderPSODesc()
{
	auto wavesRenderPSO = GetTransparentPSODesc();
	wavesRenderPSO.VS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::VSWaves)->GetBufferPointer()),
		GetShader(SShaderTypes::VSWaves)->GetBufferSize()
	};
	return wavesRenderPSO;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OEngine::GetWavesDisturbPSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC wavesPSO = {};
	wavesPSO.pRootSignature = WavesRootSignature.Get();
	wavesPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSWavesDisturb)->GetBufferPointer()),
		GetShader(SShaderTypes::CSWavesDisturb)->GetBufferSize()
	};
	wavesPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return wavesPSO;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OEngine::GetWavesUpdatePSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC wavesPSO = {};
	wavesPSO.pRootSignature = WavesRootSignature.Get();
	wavesPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSWavesUpdate)->GetBufferPointer()),
		GetShader(SShaderTypes::CSWavesUpdate)->GetBufferSize()
	};
	wavesPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return wavesPSO;
}

void OEngine::BuildPSOs()
{
	CreatePSO(SPSOType::Opaque, GetOpaquePSODesc());
	CreatePSO(SPSOType::Debug, GetDebugPSODesc());
	CreatePSO(SPSOType::Transparent, GetTransparentPSODesc());
	CreatePSO(SPSOType::AlphaTested, GetAlphaTestedPSODesc());
	CreatePSO(SPSOType::StencilMirrors, GetMirrorPSODesc());
	CreatePSO(SPSOType::StencilReflection, GetReflectedPSODesc());
	CreatePSO(SPSOType::Shadow, GetShadowPSODesc());
	CreatePSO(SPSOType::Composite, GetCompositePSODesc());
	CreatePSO(SPSOType::SobelFilter, GetSobelPSODesc());

	CreatePSO(SPSOType::WavesDisturb, GetWavesDisturbPSODesc());
	CreatePSO(SPSOType::WavesUpdate, GetWavesUpdatePSODesc());
	CreatePSO(SPSOType::WavesRender, GetWavesRenderPSODesc());

	BuildBlurPSO();
	CreatePSO(SPSOType::BilateralBlur, GetBilateralBlurPSODesc());
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OEngine::GetBilateralBlurPSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC bilateralBlurPSO = {};
	bilateralBlurPSO.pRootSignature = BilateralBlurRootSignature.Get();
	bilateralBlurPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSBilateralBlur)->GetBufferPointer()),
		GetShader(SShaderTypes::CSBilateralBlur)->GetBufferSize()
	};
	bilateralBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return bilateralBlurPSO;
}

void OEngine::BuildBlurPSO()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC horizontalBlurPSO = {};
	horizontalBlurPSO.pRootSignature = BlurRootSignature.Get();
	horizontalBlurPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSHorizontalBlur)->GetBufferPointer()),
		GetShader(SShaderTypes::CSHorizontalBlur)->GetBufferSize()
	};
	horizontalBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	CreatePSO(SPSOType::HorizontalBlur, horizontalBlurPSO);

	D3D12_COMPUTE_PIPELINE_STATE_DESC verticalBlurPSO = {};
	verticalBlurPSO.pRootSignature = BlurRootSignature.Get();
	verticalBlurPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSVerticalBlur)->GetBufferPointer()),
		GetShader(SShaderTypes::CSVerticalBlur)->GetBufferSize()
	};
	verticalBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	CreatePSO(SPSOType::VerticalBlur, verticalBlurPSO);
}

void OEngine::BuildShadersAndInputLayouts()
{
	constexpr D3D_SHADER_MACRO fogDefines[] = {
		"FOG", "1", NULL, NULL
	};

	constexpr D3D_SHADER_MACRO alphaTestDefines[] = {
		"FOG", "1", "ALPHA_TEST", "1", NULL, NULL
	};

	constexpr D3D_SHADER_MACRO wavesDefines[] = {
		"DISPLACEMENT_MAP", "1", NULL, NULL
	};

	BuildGSShader(L"Shaders/Geosphere.hlsl", SShaderTypes::GSIcosahedron);
	BuildPSShader(L"Shaders/Geosphere.hlsl", SShaderTypes::PSIcosahedron);
	BuildVSShader(L"Shaders/Geosphere.hlsl", SShaderTypes::VSIcosahedron);

	BuildVSShader(L"Shaders/BaseShader.hlsl", SShaderTypes::VSBaseShader);
	BuildPSShader(L"Shaders/BaseShader.hlsl", SShaderTypes::PSOpaque, fogDefines);
	BuildPSShader(L"Shaders/BaseShader.hlsl", SShaderTypes::PSAlphaTested, alphaTestDefines);

	BuildShader(L"Shaders/BaseShader.hlsl", SShaderTypes::VSWaves, EShaderLevel::VertexShader, "VS", wavesDefines);

	BuildVSShader(L"Shaders/TreeSprite.hlsl", SShaderTypes::VSTreeSprite);
	BuildGSShader(L"Shaders/TreeSprite.hlsl", SShaderTypes::GSTreeSprite);
	BuildPSShader(L"Shaders/TreeSprite.hlsl", SShaderTypes::PSTreeSprite);

	BuildShader(L"Shaders/Blur.hlsl", SShaderTypes::CSHorizontalBlur, EShaderLevel::ComputeShader, "HorzBlurCS");
	BuildShader(L"Shaders/Blur.hlsl", SShaderTypes::CSVerticalBlur, EShaderLevel::ComputeShader, "VertBlurCS");

	BuildShader(L"Shaders/Sobel.hlsl", SShaderTypes::CSSobelFilter, EShaderLevel::ComputeShader, "SobelCS");

	BuildShader(L"Shaders/Composite.hlsl", SShaderTypes::VSComposite, EShaderLevel::VertexShader);
	BuildShader(L"Shaders/Composite.hlsl", SShaderTypes::PSComposite, EShaderLevel::PixelShader);

	BuildShader(L"Shaders/WaveSimulation.hlsl", SShaderTypes::CSWavesDisturb, EShaderLevel::ComputeShader, "DisturbWavesCS");
	BuildShader(L"Shaders/WaveSimulation.hlsl", SShaderTypes::CSWavesUpdate, EShaderLevel::ComputeShader, "UpdateWavesCS");

	BuildShader(L"Shaders/BilateralBlur.hlsl", SShaderTypes::CSBilateralBlur, EShaderLevel::ComputeShader, "BilateralBlur");

	BuildShader(L"Shaders/BezierTesselation.hlsl", SShaderTypes::VSTesselation, EShaderLevel::VertexShader);
	BuildShader(L"Shaders/BezierTesselation.hlsl", SShaderTypes::HSTesselation, EShaderLevel::HullShader);
	BuildShader(L"Shaders/BezierTesselation.hlsl", SShaderTypes::DSTesselation, EShaderLevel::DomainShader);
	BuildShader(L"Shaders/BezierTesselation.hlsl", SShaderTypes::PSTesselation, EShaderLevel::PixelShader);
	InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
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

vector<SRenderItem*>& OEngine::GetRenderItems(const string& Type)
{
	return RenderLayers[Type];
}

ComPtr<IDXGIFactory2> OEngine::GetFactory() const
{
	return Factory;
}

void OEngine::AddMaterial(string Name, unique_ptr<SMaterial>& Material)
{
	if (Materials.contains(Name))
	{
		LOG(Engine, Error, "Material with this name already exists!");
		return;
	}
	Materials[Name] = move(Material);
}

void OEngine::CreateMaterial(const string& Name, const int32_t CBIndex, const int32_t HeapIdx, const SMaterialData& Constants)
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
		LOG(Engine, Error, "Material not found!");
		return nullptr;
	}
	return Materials.at(Name).get();
}

STexture* OEngine::CreateTexture(string Name, wstring FileName)
{
	if (Textures.contains(Name))
	{
		LOG(Engine, Error, "Texture with this name already exists!");
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
		LOG(Engine, Error, "Texture not found!");
		return nullptr;
	}
	return Textures.at(Name).get();
}

void OEngine::AddRenderItem(string Category, unique_ptr<SRenderItem> RenderItem)
{
	RenderLayers[Category].push_back(RenderItem.get());
	AllRenderItems.push_back(move(RenderItem));
}

void OEngine::AddRenderItem(const vector<string>& Categories, unique_ptr<SRenderItem> RenderItem)
{
	for (auto category : Categories)
	{
		RenderLayers[category].push_back(RenderItem.get());
	}
	AllRenderItems.push_back(move(RenderItem));
}

const vector<unique_ptr<SRenderItem>>& OEngine::GetAllRenderItems()
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

OBlurFilter* OEngine::GetBlurFilter()
{
	return GetObjectByUUID<OBlurFilter>(BlurFilterUUID);
}

OBilateralBlurFilter* OEngine::GetBilateralBlurFilter()
{
	return GetObjectByUUID<OBilateralBlurFilter>(BilateralFilterUUID);
}

OSobelFilter* OEngine::GetSobelFilter()
{
	return GetObjectByUUID<OSobelFilter>(SobelFilterUUID);
}

SRenderObjectDescriptor OEngine::GetObjectDescriptor()
{
	const auto rtvDesc = Window->RTVHeap->GetCPUDescriptorHandleForHeapStart();
	SRenderObjectDescriptor desc;
	desc.CPUSRVescriptor = SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	desc.GPUSRVDescriptor = SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	desc.DSVSRVUAVDescriptorSize = CBVSRVUAVDescriptorSize;
	desc.CPURTVDescriptor = rtvDesc;
	desc.RTVDescriptorSize = RTVDescriptorSize;
	return desc;
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
		LOG(Engine, Error, "Test not found!");
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
	LOG(Engine, Error, "Window not found!");

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

void OEngine::BuildPostProcessRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable0;
	srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable1;
	srvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	//Order from most frequent to least frequent.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	slotRootParameter[0].InitAsDescriptorTable(1, &srvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable0);

	auto staticSamplers = Utils::GetStaticSamplers();

	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3,
	                                              slotRootParameter,
	                                              staticSamplers.size(),
	                                              staticSamplers.data(),
	                                              D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Utils::BuildRootSignature(Device.Get(), PostProcessRootSignature, rootSigDesc);
}

void OEngine::BuildDefaultRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE displacementTable;
	displacementTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0);

	constexpr auto size = 5;
	CD3DX12_ROOT_PARAMETER slotRootParameter[size];

	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);

	slotRootParameter[2].InitAsConstantBufferView(0);

	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsDescriptorTable(1, &displacementTable, D3D12_SHADER_VISIBILITY_VERTEX);

	const auto staticSamples
	    = Utils::GetStaticSamplers();
	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(size, slotRootParameter, staticSamples.size(), staticSamples.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Utils::BuildRootSignature(Device.Get(), DefaultRootSignature, rootSigDesc);
}

void OEngine::BuildBilateralBlurRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable0;
	srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	slotRootParameter[0].InitAsConstants(3, 0);
	slotRootParameter[1].InitAsConstants(2, 1);
	slotRootParameter[2].InitAsDescriptorTable(1, &srvTable0);
	slotRootParameter[3].InitAsDescriptorTable(1, &uavTable1);

	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4,
	                                              slotRootParameter,
	                                              0,
	                                              nullptr,
	                                              D3D12_ROOT_SIGNATURE_FLAG_NONE);

	Utils::BuildRootSignature(Device.Get(), BilateralBlurRootSignature, rootSigDesc);
}

void OEngine::BuildBlurRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable0;
	srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	slotRootParameter[0].InitAsConstants(12, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable0);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable1);

	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3,
	                                              slotRootParameter,
	                                              0,
	                                              nullptr,
	                                              D3D12_ROOT_SIGNATURE_FLAG_NONE);

	Utils::BuildRootSignature(Device.Get(), BlurRootSignature, rootSigDesc);
}

void OEngine::BuildWavesRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable2;
	uavTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	slotRootParameter[0].InitAsConstants(6, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &uavTable0);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable1);
	slotRootParameter[3].InitAsDescriptorTable(1, &uavTable2);

	const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4,
	                                              slotRootParameter,
	                                              0,
	                                              nullptr,
	                                              D3D12_ROOT_SIGNATURE_FLAG_NONE);

	Utils::BuildRootSignature(Device.Get(), WavesRootSignature, rootSigDesc);
}

uint32_t OEngine::GetNumOffscrenRT() const
{
	//TODO move RT to array
	return 1;
}

OMeshGenerator* OEngine::GetMeshGenerator() const
{
	return MeshGenerator.get();
}

float OEngine::GetDeltaTime() const
{
	return TickTimer.GetDeltaTime();
}

OEngine::TRenderLayer& OEngine::GetRenderLayers()
{
	return RenderLayers;
}

void OEngine::PerformFrustrumCulling()
{
	if (CurrentFrameResources == nullptr)
	{
		return;
	}

	const auto view = Window->GetCamera()->GetView();
	auto det = XMMatrixDeterminant(view);
	const auto invView = XMMatrixInverse(&det, view);
	const auto camera = Window->GetCamera();
	auto currentInstanceBuffer = CurrentFrameResources->InstanceBuffer.get();
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
			auto world = XMLoadFloat4x4(&instData[i].World);
			auto textTransform = XMLoadFloat4x4(&instData[i].TexTransform);
			det = XMMatrixDeterminant(world);
			auto invWorld = XMMatrixInverse(&det, world);

			auto viewToLocal = XMMatrixMultiply(invView, invWorld);
			BoundingFrustum localSpaceFrustum;
			camera->GetFrustrum().Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			if (localSpaceFrustum.Contains(e->Bounds) != DirectX::DISJOINT || !FrustrumCullingEnabled)
			{
				SInstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(textTransform));
				data.MaterialIndex = instData[i].MaterialIndex;
				currentInstanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}
		e->VisibleInstanceCount = visibleInstanceCount;
		LOG(Render, Log, "Number of visible instances of object {} out of {}", visibleInstanceCount, e->Instances.size());
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
	GetCommandQueue()->ResetCommandList();

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

void OEngine::SetLightSources(const vector<SLight>& Lights)
{
	for (int32_t i = 0; i < Lights.size(); i++)
	{
		if (SRenderConstants::MaxLights > i)
		{
			MainPassCB.Lights[i] = Lights[i];
		}
	}
}
void OEngine::SetAmbientLight(const DirectX::XMFLOAT3& Color)
{
	MainPassCB.AmbientLight = XMFLOAT4(Color.x, Color.y, Color.z, 1);
}

void OEngine::UpdateMainPass(const STimer& Timer)
{
	const XMMATRIX view = Window->GetCamera()->GetView();
	const XMMATRIX proj = Window->GetCamera()->GetProj();
	const XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	auto viewDet = XMMatrixDeterminant(view);
	auto projDet = XMMatrixDeterminant(proj);
	auto viewProjDet = XMMatrixDeterminant(viewProj);

	const XMMATRIX invView = XMMatrixInverse(&viewDet, view);
	const XMMATRIX invProj = XMMatrixInverse(&projDet, proj);
	const XMMATRIX invViewProj = XMMatrixInverse(&viewProjDet, viewProj);

	XMStoreFloat4x4(&MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	MainPassCB.EyePosW = Window->GetCamera()->GetPosition3f();
	MainPassCB.RenderTargetSize = XMFLOAT2(static_cast<float>(Window->GetWidth()), static_cast<float>(Window->GetHeight()));
	MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / Window->GetWidth(), 1.0f / Window->GetHeight());
	MainPassCB.NearZ = 1.0f;
	MainPassCB.FarZ = 1000.0f;
	MainPassCB.TotalTime = Timer.GetTime();
	MainPassCB.DeltaTime = Timer.GetDeltaTime();

	const auto currPassCB = CurrentFrameResources->PassCB.get();
	currPassCB->CopyData(0, MainPassCB);
}

std::unordered_map<string, unique_ptr<SMeshGeometry>>& OEngine::GetSceneGeometry()
{
	return SceneGeometry;
}

void OEngine::SetSceneGeometry(unique_ptr<SMeshGeometry> Geometry)
{
	SceneGeometry[Geometry->Name] = move(Geometry);
}

void OEngine::BuildRenderItemFromMesh(string Category, unique_ptr<SMeshGeometry> Mesh, std::vector<SMaterial*>* InstanceMaterialArray)
{
	SRenderItem newItem;
	newItem.World = Utils::Math::Identity4x4();
	newItem.TexTransform = Utils::Math::Identity4x4();
	newItem.ObjectCBIndex = AllRenderItems.size();
	newItem.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	if (InstanceMaterialArray != nullptr)
	{
		newItem.Instances.resize(InstanceMaterialArray->size());
		size_t counter = 0;
		for (const auto& material : *InstanceMaterialArray)
		{
			SInstanceData data = {};
			data.MaterialIndex = material->MaterialCBIndex;
			newItem.Instances[counter] = data;
			counter++;
		}
	}
	else
	{
		SInstanceData data = {};
		data.MaterialIndex = 0;
		newItem.Instances.push_back(data);
	}

	for (auto& subMesh : Mesh->GetDrawArgs() | std::views::values)
	{
		newItem.IndexCount = subMesh.IndexCount;
		newItem.StartIndexLocation = subMesh.StartIndexLocation;
		newItem.BaseVertexLocation = subMesh.BaseVertexLocation;
		newItem.Bounds = subMesh.Bounds;
		auto item = make_unique<SRenderItem>(newItem);
		AddRenderItem(Category, std::move(item));
	}
}
vector<SInstanceData>& OEngine::BuildRenderItemFromMesh(string Category, SMeshGeometry* Mesh, size_t NumberOfInstances, string Submesh)
{
	auto newItem = make_unique<SRenderItem>();
	newItem->World = Utils::Math::Identity4x4();
	newItem->TexTransform = Utils::Math::Identity4x4();
	newItem->ObjectCBIndex = AllRenderItems.size();
	newItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	newItem->Instances.resize(NumberOfInstances);
	newItem->RenderLayer = Category;
	newItem->Geometry = Mesh;
	const auto itemptr = newItem.get();

	if (Submesh == "")
	{
		Submesh = Mesh->GetDrawArgs().begin()->first;
	}
	const auto& chosenSubmesh = *Mesh->FindSubmeshGeomentry(Submesh);
	newItem->IndexCount = chosenSubmesh.IndexCount;
	newItem->StartIndexLocation = chosenSubmesh.StartIndexLocation;
	newItem->BaseVertexLocation = chosenSubmesh.BaseVertexLocation;
	newItem->Bounds = chosenSubmesh.Bounds;
	AddRenderItem(Category, std::move(newItem));
	return itemptr->Instances;
}
vector<SInstanceData>& OEngine::BuildRenderItemFromMesh(const string& Category, const string& Name, const string& Path, const EParserType Parser, ETextureMapType GenTexels, size_t NumberOfInstances)
{
	auto mesh = MeshGenerator->CreateMesh(Name, Path, Parser, GenTexels, Device.Get(), GetCommandQueue()->GetCommandList().Get());
	const auto meshptr = mesh.get();
	SetSceneGeometry(std::move(mesh));
	return BuildRenderItemFromMesh(Category, meshptr, NumberOfInstances, {});
}

unique_ptr<SMeshGeometry> OEngine::CreateMesh(const string& Name, const string& Path, const EParserType Parser, ETextureMapType GenTexels)
{
	return MeshGenerator->CreateMesh(Name, Path, Parser, GenTexels, Device.Get(), GetCommandQueue()->GetCommandList().Get());
}

unique_ptr<SMeshGeometry> OEngine::CreateMesh(const string& Name, const OGeometryGenerator::SMeshData& Data)
{
	return MeshGenerator->CreateMesh(Name, Data, Device.Get(), GetCommandQueue()->GetCommandList().Get());
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

void OEngine::BuildGSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines)
{
	Shaders[ShaderName] = Utils::CompileShader(ShaderPath, Defines, "GS", "gs_5_1");
}

void OEngine::BuildCSShader(const wstring& ShaderPath, const string& ShaderName, const D3D_SHADER_MACRO* Defines)
{
	Shaders[ShaderName] = Utils::CompileShader(ShaderPath, Defines, "CS", "cs_5_1");
}

void OEngine::BuildShader(const wstring& ShaderPath, const string& ShaderName, EShaderLevel ShaderType, std::optional<string> ShaderEntry, const D3D_SHADER_MACRO* Defines)
{
	string entry = "";
	string target = "";
	switch (ShaderType)
	{
	case EShaderLevel::VertexShader:
		entry = "VS";
		target = "vs_5_1";
		break;
	case EShaderLevel::PixelShader:
		entry = "PS";
		target = "ps_5_1";
		break;
	case EShaderLevel::GeometryShader:
		entry = "GS";
		target = "gs_5_1";
		break;
	case EShaderLevel::HullShader:
		entry = "HS";
		target = "hs_5_1";
		break;
	case EShaderLevel::DomainShader:
		entry = "DS";
		target = "ds_5_1";
		break;
	case EShaderLevel::ComputeShader:
		entry = "CS";
		target = "cs_5_1";
		break;
	}
	entry = ShaderEntry.value_or(entry);
	Shaders[ShaderName] = Utils::CompileShader(ShaderPath, Defines, entry, target);
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

ComPtr<ID3D12DescriptorHeap>& OEngine::GetSRVHeap()
{
	return SRVDescriptorHeap;
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
