#pragma once
#include "Events.h"

#include <Types.h>

class OTest
{
public:
	virtual ~OTest() = default;
	OTest() = default;
	explicit OTest(const shared_ptr<class OEngine>& _Engine)
	    : Engine(_Engine)
	{
	}

	virtual void LoadContent() = 0;
	virtual void UnloadContent() = 0;
	virtual void Destroy() = 0;
	virtual void Update() = 0;
	virtual void OnUpdate(UpdateEventArgs& e) = 0;
	virtual void OnRender() = 0;
	virtual void OnResize(ResizeEventArgs& e) = 0;
	virtual void OnWindowDestroyed() = 0;
	virtual void OnKeyPressed(KeyEventArgs& e) = 0;
	virtual void OnMouseWheel(MouseMotionEventArgs& e) = 0;

protected:
	weak_ptr<OEngine> Engine;
};
