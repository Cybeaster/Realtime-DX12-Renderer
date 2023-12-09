#include "Test.h"

#include "Application.h"

void OTest::Destroy()
{
	Engine.lock()->DestroyWindow();
}

void OTest::Update()
{
}
void OTest::OnUpdate(UpdateEventArgs& e)
{
}

void OTest::OnResize(ResizeEventArgs& Args)
{
	ClientWidth = Args.Width;
	ClientHeight = Args.Height;
}
void OTest::OnWindowDestroyed()
{
	UnloadContent();
}
void OTest::OnKeyPressed(KeyEventArgs& e)
{
}
