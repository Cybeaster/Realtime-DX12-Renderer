#include "Application.h"

#include "Engine/Engine.h"
#include "Test/SimpleCubeTest/SimpleCubeTest.h"

#include <DirectXMath.h>

void OApplication::InitApplication()
{
	Engine = make_shared<OEngine>(L"Test", 800, 600, false);
	Engine->Initialize();

	Tests.push_back(make_shared<OSimpleCubeTest>(Engine));
}
