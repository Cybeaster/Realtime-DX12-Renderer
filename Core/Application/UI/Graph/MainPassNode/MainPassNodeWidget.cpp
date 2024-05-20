//
// Created by Cybea on 18/05/2024.
//

#include "MainPassNodeWidget.h"

#include "Engine/Engine.h"
#include "Window/Window.h"
void OOpaquePassNodeWidget::Draw()
{
	ORenderNodeWidgetBase::Draw();
	ImGui::SeparatorText("Main Pass Node Info");
	ImGui::Image(PtrCast(OEngine::Get()->GetWindow().lock()->GetSRVDepth().GPUHandle.ptr), ImVec2(250, 250));
}