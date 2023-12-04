#pragma once

#include "Window/Window.h"

#include <Types.h>

class OTest;
class OApplication
{
public:
	static OApplication* Get()
	{
		if (Application == nullptr)
		{
			Application = new OApplication();
		}
		return Application;
	}

	auto CreateWindow(wstring Name, uint32_t Width, uint32_t Height, bool VSync)
	{
		return make_shared<OWindow>(Name, Width, Height, VSync);
	}

	void DestroyWindow(shared_ptr<OWindow> Window)
	{
		Window.reset();
	}

	void InitApplication();

private:
	OApplication();

	static OApplication* Application;
	shared_ptr<OEngine> Engine;
	vector<shared_ptr<OTest>> Tests;
};
