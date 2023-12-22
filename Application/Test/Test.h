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
	explicit OTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
	    : Engine(_Engine), Window(_Window)
	{
	}
	virtual bool Initialize() { return true; }
	virtual void UnloadContent() {}
	virtual void Destroy();

	virtual void OnUpdate(const UpdateEventArgs& Args) {}
	virtual void OnRender(const UpdateEventArgs& Arg) = 0;

	virtual void OnWindowDestroyed();

	virtual void OnResize(const ResizeEventArgs& Args) {}
	virtual void OnKeyPressed(const KeyEventArgs& Args) {}
	virtual void OnMouseWheel(const MouseWheelEventArgs& Args) {}
	virtual void OnMouseMoved(const MouseMotionEventArgs& Args) {}
	virtual void OnMouseButtonPressed(const MouseButtonEventArgs& Args) {}
	virtual void OnMouseButtonReleased(const MouseButtonEventArgs& Args) {}

	auto GetWindow() const
	{
		return Window.lock();
	}

protected:
	weak_ptr<OEngine> Engine;
	weak_ptr<OWindow> Window;
};
