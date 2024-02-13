#pragma once

#include "Events.h"
#include "Types.h"
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

class OHierarchicalWidgetBase : public IWidget
{
public:
	template<typename WidgetType, typename... Params>
	WidgetType* MakeWidget(Params&&... Args)
	{
		auto newWidget = make_unique<WidgetType>(std::forward<Params>(Args)...);
		newWidget->Init();
		auto result = newWidget.get();
		Widgets.push_back(move(newWidget));
		return result;
	}

	std::vector<std::unique_ptr<IWidget>>& GetWidgets()
	{
		return Widgets;
	}

	void Draw() override
	{
		for (const auto& widget : GetWidgets())
		{
			widget->Draw();
		}
	}

	void Update() override
	{
		for (const auto& widget : GetWidgets())
		{
			widget->Update();
		}
	}

private:
	std::vector<std::unique_ptr<IWidget>> Widgets;
};