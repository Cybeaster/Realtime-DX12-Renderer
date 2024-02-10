#include "Test.h"

#include "Application.h"

void OTest::Destroy()
{
	Engine->DestroyWindow();
}

void OTest::OnWindowDestroyed()
{
	UnloadContent();
}
