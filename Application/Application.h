#pragma once

#include "Engine/Engine.h"
#include "Timer/Timer.h"

#include <Types.h>

class OTest;
class OApplication
{
public:
	static OApplication* Get();
	void Destory();

	shared_ptr<OWindow> CreateWindow();

	void DestroyWindow(shared_ptr<OWindow> Window)
	{
		Window.reset();
	}

	void Quit(int ExitCode);
	void InitApplication(HINSTANCE hInstance);

	template<typename TestType = OTest>
	int Run();

	inline static wchar_t WindowClassName[] = L"DXRendererClass";

	HINSTANCE GetAppInstance() const;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void SetAppPaused(bool bPaused);

private:
	OApplication();
	void InitWindowClass() const;
	void CalculateFrameStats();

	inline static OApplication* Application = nullptr;
	HINSTANCE AppInstance = nullptr;

	inline static shared_ptr<OEngine> Engine = nullptr;
	vector<shared_ptr<OTest>> Tests;

	STimer Timer;
	bool bIsAppPaused = false;
	bool bIsAppMinimized = false;
	bool bIsAppMaximized = false;
	bool bIsResizing = false;

	SWindowInfo DefaultWindowInfo = { false, L"Window", 800, 600, false, 45.f };
};

template<typename TestType>
int OApplication::Run()
{
	auto test = make_shared<TestType>(Engine, Engine->GetWindow());
	Engine->Run(test);
	MSG msg = { 0 };
	Timer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Timer.Tick();
			CalculateFrameStats();
			if (bIsAppPaused)
			{
				Sleep(100);
			}
			else
			{
				UpdateEventArgs args(Timer, Engine->GetWindow()->GetHWND());
				Engine->Draw(args);
			}
		}
	}
	Engine->OnEnd(test);
	return static_cast<int>(msg.wParam);
}
