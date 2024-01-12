#include "Test.h"

#include "Application.h"

void OTest::Destroy()
{
	Engine.lock()->DestroyWindow();
}

void OTest::OnWindowDestroyed()
{
	UnloadContent();
}


