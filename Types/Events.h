#pragma once
#include "KeyCodes.h"
#include "Timer/Timer.h"

#include <Types.h>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>
// Super class for all event args
class EventArgs
{
public:
	EventArgs(HWND Handle)
	    : WindowHandle(Handle)
	{
	}

	HWND WindowHandle;
};

class KeyEventArgs : public EventArgs
{
public:
	enum KeyState
	{
		Released = 0,
		Pressed = 1
	};

	typedef EventArgs Super;
	KeyEventArgs(KeyCode::Key key, unsigned int c, KeyState state, bool control, bool shift, bool alt, HWND Handle)
	    : Super(Handle), Key(key), Char(c), State(state), Control(control), Shift(shift), Alt(alt)
	{
	}

	KeyCode::Key Key; // The Key Code that was pressed or released.
	unsigned int Char; // The 32-bit character code that was pressed. This value will be 0 if it is a non-printable character.
	KeyState State; // Was the key pressed or released?
	bool Control; // Is the Control modifier pressed
	bool Shift; // Is the Shift modifier pressed
	bool Alt; // Is the Alt modifier pressed
};

class MouseMotionEventArgs : public EventArgs
{
public:
	typedef EventArgs Super;
	MouseMotionEventArgs(bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y, HWND Handle)
	    : Super(Handle), LeftButton(leftButton), MiddleButton(middleButton), RightButton(rightButton), Control(control), Shift(shift), X(x), Y(y)
	{
	}

	bool LeftButton; // Is the left mouse button down?
	bool MiddleButton; // Is the middle mouse button down?
	bool RightButton; // Is the right mouse button down?
	bool Control; // Is the CTRL key down?
	bool Shift; // Is the Shift key down?

	int X; // The X-position of the cursor relative to the upper-left corner of the client area.
	int Y; // The Y-position of the cursor relative to the upper-left corner of the client area.
};

class MouseButtonEventArgs : public EventArgs
{
public:
	enum MouseButton
	{
		None = 0,
		Left = 1,
		Right = 2,
		Middel = 3
	};
	enum ButtonState
	{
		Released = 0,
		Pressed = 1
	};

	typedef EventArgs Super;
	MouseButtonEventArgs(MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y, HWND Handle)
	    : Super(Handle), Button(buttonID), State(state), LeftButton(leftButton), MiddleButton(middleButton), RightButton(rightButton), Control(control), Shift(shift), X(x), Y(y)
	{
	}

	MouseButton Button; // The mouse button that was pressed or released.
	ButtonState State; // Was the button pressed or released?
	bool LeftButton; // Is the left mouse button down?
	bool MiddleButton; // Is the middle mouse button down?
	bool RightButton; // Is the right mouse button down?
	bool Control; // Is the CTRL key down?
	bool Shift; // Is the Shift key down?

	int X; // The X-position of the cursor relative to the upper-left corner of the client area.
	int Y; // The Y-position of the cursor relative to the upper-left corner of the client area.
};

class MouseWheelEventArgs : public EventArgs
{
public:
	typedef EventArgs Super;
	MouseWheelEventArgs(float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y, HWND Handle)
	    : Super(Handle), WheelDelta(wheelDelta), LeftButton(leftButton), MiddleButton(middleButton), RightButton(rightButton), Control(control), Shift(shift), X(x), Y(y)
	{
	}

	float WheelDelta; // How much the mouse wheel has moved. A positive value indicates that the wheel was moved to the right. A negative value indicates the wheel was moved to the left.
	bool LeftButton; // Is the left mouse button down?
	bool MiddleButton; // Is the middle mouse button down?
	bool RightButton; // Is the right mouse button down?
	bool Control; // Is the CTRL key down?
	bool Shift; // Is the Shift key down?

	int X; // The X-position of the cursor relative to the upper-left corner of the client area.
	int Y; // The Y-position of the cursor relative to the upper-left corner of the client area.
};

class ResizeEventArgs : public EventArgs
{
public:
	typedef EventArgs Super;
	ResizeEventArgs(int width, int height, HWND Handle)
	    : Super(Handle), Width(width), Height(height)
	{
	}

	// The new width of the window
	int Width;
	// The new height of the window.
	int Height;
};

class UpdateEventArgs : public EventArgs
{
public:
	typedef EventArgs Super;
	UpdateEventArgs(const STimer& Other, HWND Handle)
	    : Super(Handle), Timer(Other)
	{
	}
	const STimer& Timer;
};

class UserEventArgs : public EventArgs
{
public:
	typedef EventArgs Super;
	UserEventArgs(int code, void* data1, void* data2, HWND Handle)
	    : Super(Handle), Code(code), Data1(data1), Data2(data2)
	{
	}

	int Code;
	void* Data1;
	void* Data2;
};