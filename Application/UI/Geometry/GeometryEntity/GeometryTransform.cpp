#pragma once

#include "GeometryTransform.h"

#include "MathUtils.h"
#include "imgui.h"

void OGeometryTransformWidget::Draw()
{
	ImGui::SeparatorText("Transform");
	if (ImGui::InputFloat3("Position", &NewPosition.x))
	{
		OnTransformUpdate.Broadcast();
	}

	ImGui::Separator();
	if (ImGui::InputFloat3("Rotation", &NewRotation.x))
	{
		OnTransformUpdate.Broadcast();
	}

	ImGui::Separator();
	if (ImGui::InputFloat3("Scale", &NewScale.x))
	{
		OnTransformUpdate.Broadcast();
	}

	*Position = NewPosition;
	*Rotation = NewRotation;
	*Scale = NewScale;
}