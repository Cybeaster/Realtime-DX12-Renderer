#pragma once

#include "GeometryTransform.h"

#include "imgui.h"

void OGeometryTransformWidget::Draw()
{
	ImGui::SeparatorText("Transform");
	DirectX::XMFLOAT3 newPosition = *Position;
	if (ImGui::SliderFloat3("Position", &newPosition.x, -100, 100))
	{
		XMStoreFloat3(Position, DirectX::XMVectorLerp(XMLoadFloat3(Position), XMLoadFloat3(&newPosition), 0.1f));
		OnTransformUpdate.Broadcast();
	}

	ImGui::Separator();
	DirectX::XMFLOAT3 newRotation = *Rotation;
	if (ImGui::SliderFloat3("Rotation", &Rotation->x, -1, 1))
	{
		XMStoreFloat3(Rotation, DirectX::XMVectorLerp(XMLoadFloat3(Rotation), XMLoadFloat3(&newRotation), 0.1f));
		OnTransformUpdate.Broadcast();
	}

	ImGui::Separator();
	DirectX::XMFLOAT3 newScale = *Scale;
	if (ImGui::SliderFloat3("Scale", &newScale.x, 0.1f, 10))
	{
		XMStoreFloat3(Scale, DirectX::XMVectorLerp(XMLoadFloat3(Scale), XMLoadFloat3(&newScale), 0.1f));

		OnTransformUpdate.Broadcast();
	}
}