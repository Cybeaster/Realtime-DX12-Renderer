#pragma once
#include "Events.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include <imgui.h>

class IWidget
{
public:
	virtual ~IWidget() = default;
	virtual void Init(){};
	virtual void Update() {}
	virtual void Draw() = 0;
	virtual bool IsInFocus() { return false; }
	virtual bool IsEnabled() { return false; }

	virtual void OnMouseButtonPressed(MouseButtonEventArgs& Args) {}
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& Args) {}
};
