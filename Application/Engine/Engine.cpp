
#include "Engine.h"

#include "../Application.h"
#include "../Test/Test.h"
#include "../Window/Window.h"
#include "Exception.h"
#include "Logger.h"

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

void OEngine::BuildFrameResource()
{
	for (int i = 0; i < SRenderConstants::NumFrameResources; ++i)
	{
		FrameResources.push_back(make_unique<SFrameResource>(Device.Get(), 1, 1));
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
	// By default, do nothing.
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

const vector<unique_ptr<SRenderItem>>& OEngine::GetRenderItems()
{
	return AllRenderItems;
}

const vector<unique_ptr<SRenderItem>>& OEngine::GetOpaqueRenderItems()
{
	return OpaqueRenderItems;
}

const vector<unique_ptr<SRenderItem>>& OEngine::GetTransparentRenderItems()
{
	return TransparentRenderItems;
}

ComPtr<IDXGIFactory2> OEngine::GetFactory() const
{
	return Factory;
}

shared_ptr<OTest> OEngine::GetTestByHWND(HWND Handler)
{
	const auto test = Tests.find(Handler);
	if (test != Tests.end())
	{
		return test->second;
	}
	LOG(Error, "Test not found!");
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