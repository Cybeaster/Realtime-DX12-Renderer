#pragma once

#include "Engine/Engine.h"
#include "Window/Window.h"

#include <Types.h>

class OTest;
class OApplication
{
public:
	static OApplication* Get();
	void Destory();
	shared_ptr<OWindow> CreateWindow(wstring Name, uint32_t Width, uint32_t Height, bool VSync);

	void DestroyWindow(shared_ptr<OWindow> Window)
	{
		Window.reset();
	}

	void Quit(int ExitCode);
	void InitApplication(HINSTANCE hInstance);

	template<typename TestType = OTest>
	int Run() const;

	inline static wchar_t WindowClassName[] = L"DXRendererClass";

	HINSTANCE GetAppInstance() const;

private:
	OApplication();

	inline static OApplication* Application = nullptr;
	HINSTANCE AppInstance = nullptr;

	shared_ptr<OEngine> Engine;
	vector<shared_ptr<OTest>> Tests;
};

template<typename TestType>
int OApplication::Run() const
{
	auto test = make_shared<TestType>(Engine);
	Engine->Run(test);
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	Engine->OnEnd(test);
	return static_cast<int>(msg.wParam);
}
