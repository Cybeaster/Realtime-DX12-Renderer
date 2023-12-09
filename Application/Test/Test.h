#pragma once
#include "Engine/Engine.h"
#include "Events.h"

#include <Types.h>

class OEngine;
class OTest
{
public:
	virtual ~OTest() = default;
	OTest() = default;
	explicit OTest(const shared_ptr<OEngine>& _Engine)
	    : Engine(_Engine), Name(_Engine->GetName()), ClientWidth(_Engine->GetWidth()), ClientHeight(_Engine->GetHeight()), VSync(_Engine->IsVSync())
	{
	}
	virtual void LoadContent() = 0;
	virtual void UnloadContent() = 0;
	virtual void Destroy();
	virtual void Update();
	virtual void OnUpdate(UpdateEventArgs& e);
	virtual void OnRender() = 0;
	virtual void OnResize(ResizeEventArgs& Args);
	virtual void OnWindowDestroyed();
	virtual void OnKeyPressed(KeyEventArgs& e);
	virtual void OnMouseWheel(MouseWheelEventArgs& e) = 0;

protected:
	weak_ptr<OEngine> Engine;
	wstring Name;
	uint32_t ClientWidth;
	uint32_t ClientHeight;
	bool VSync;
};