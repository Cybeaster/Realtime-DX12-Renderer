
#include "Engine.h"

#include "../Application.h"
#include "../Window/Window.h"
#include "Logger.h"

#include <DirectXMath.h>

bool OEngine::Initialize()
{
	if (DirectX::XMVerifyCPUSupport())
	{
		MessageBox(nullptr, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}
	Window = OApplication::Get()->CreateWindow(Name, Width, Height, VSync);
	Window->RegisterInput(shared_from_this());

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

	return true;
}

shared_ptr<OWindow> OEngine::GetWindow() const
{
	return Window;
}

const wstring& OEngine::GetName() const
{
	return Name;
}

uint32_t OEngine::GetWidth() const
{
	return Width;
}

uint32_t OEngine::GetHeight() const
{
	return Height;
}

Microsoft::WRL::ComPtr<ID3D12Device2> OEngine::GetDevice() const
{
	return Device;
}

void OEngine::FlushGPU()
{
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

void OEngine::Destroy()
{
	OApplication::Get()->DestroyWindow(Window);
}
