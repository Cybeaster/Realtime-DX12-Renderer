#pragma once
#include "Events.h"

class OInputHandler
{
public:
	virtual ~OInputHandler() = default;

protected:
	/**
	 * Invoked by the registered window when a key is pressed
	 * while the window has focus.
	 */
	virtual void OnKeyPressed(KeyEventArgs& e);

	/**
	 * Invoked when a key on the keyboard is released.
	 */
	virtual void OnKeyReleased(KeyEventArgs& e);

	/**
	 * Invoked when the mouse is moved over the registered window.
	 */
	virtual void OnMouseMoved(MouseMotionEventArgs& e);

	/**
	 * Invoked when a mouse button is pressed over the registered window.
	 */
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);

	/**
	 * Invoked when a mouse button is released over the registered window.
	 */
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

	/**
	 * Invoked when the mouse wheel is scrolled while the registered window has focus.
	 */
	virtual void OnMouseWheel(MouseWheelEventArgs& e);
};
