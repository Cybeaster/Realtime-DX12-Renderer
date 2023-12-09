#include "Application.h"

#include "Engine/Engine.h"
#include "Exception.h"
#include "Test/SimpleCubeTest/SimpleCubeTest.h"

#include <DirectXMath.h>

using namespace Microsoft::WRL;
OApplication* OApplication::Get()
{
	if (Application == nullptr)
	{
		Application = new OApplication();
	}
	return Application;
}

void OApplication::Destory()
{
	delete Application;
	Application = nullptr;
}

shared_ptr<OWindow> OApplication::CreateWindow(wstring Name, uint32_t Width, uint32_t Height, bool VSync)
{




	RECT windowRect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindowW(WindowClassName,
	                          Name.c_str(),
	                          WS_OVERLAPPEDWINDOW,
	                          CW_USEDEFAULT,
	                          CW_USEDEFAULT,
	                          windowRect.right - windowRect.left,
	                          windowRect.bottom - windowRect.top,
	                          nullptr,
	                          nullptr,
	                          AppInstance,
	                          nullptr);
	if (!hWnd)
	{
		MessageBoxA(NULL, "Failed to create window.", "Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return make_shared<OWindow>(Engine, hWnd, Name, Width, Height, VSync);
}

void OApplication::Quit(int ExitCode)
{
	PostQuitMessage(ExitCode);
}

void OApplication::InitApplication(HINSTANCE hInstance)
{
	AppInstance = hInstance;

	Engine = make_shared<OEngine>(L"Test", 800, 600, false);
	Engine->Initialize();
}

HINSTANCE OApplication::GetAppInstance() const
{
	return AppInstance;
}

OApplication::OApplication()
{
}