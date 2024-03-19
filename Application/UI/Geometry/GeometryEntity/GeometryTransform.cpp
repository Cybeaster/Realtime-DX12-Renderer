#pragma once

#include "GeometryTransform.h"

#include "imgui.h"

void OGeometryTransformWidget::Draw()
{
	ImGui::SeparatorText("Transform");
	DirectX::XMFLOAT3 newPosition = *Position;
	if (ImGui::InputFloat3("Position", &newPosition.x))
	{
		XMStoreFloat3(Position, DirectX::XMVectorLerp(XMLoadFloat3(Position), XMLoadFloat3(&newPosition), 0.1f));
		OnTransformUpdate.Broadcast();
	}

	ImGui::Separator();
	DirectX::XMFLOAT3 newRotation = *Rotation;
	if (ImGui::InputFloat3("Rotation", &Rotation->x))
	{
		XMStoreFloat3(Rotation, DirectX::XMVectorLerp(XMLoadFloat3(Rotation), XMLoadFloat3(&newRotation), 0.1f));
		OnTransformUpdate.Broadcast();
	}

	ImGui::Separator();
	DirectX::XMFLOAT3 newScale = *Scale;
	if (ImGui::InputFloat3("Scale", &newScale.x))
	{
		XMStoreFloat3(Scale, DirectX::XMVectorLerp(XMLoadFloat3(Scale), XMLoadFloat3(&newScale), 0.1f));

		OnTransformUpdate.Broadcast();
	}
}