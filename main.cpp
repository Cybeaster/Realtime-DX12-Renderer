#define WIN32_LEAN_AND_MEAN

#define _DEBUG

#include "Test/LandTest/LandTest.h"
#include "Test/LitWaves/LitWaves.h"
#include "Test/TextureTest/TextureWaves.h"


#include <Application.h>
#include <Shlwapi.h>
#include <Windows.h>

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	int returnCode = 0;

	const HMODULE hModule = GetModuleHandleW(NULL);

	if (WCHAR path[MAX_PATH]; GetModuleFileNameW(hModule, path, MAX_PATH) > 0)
	{
		PathRemoveFileSpecW(path);
		SetCurrentDirectoryW(path);
	}

	const auto application = OApplication::Get();
	application->InitApplication(hInstance);

	returnCode = application->Run<OTextureWaves>();
	application->Destory();

	return returnCode;
}
