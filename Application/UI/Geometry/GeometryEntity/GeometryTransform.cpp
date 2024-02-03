#pragma once

#include "GeometryTransform.h"

void OGeometryTransformWidget::Draw()
{
	ImGui::SeparatorText("Transform");
	DirectX::XMFLOAT3 newPosition = *Position;
	if (ImGui::SliderFloat3("Position", &newPosition.x, -100, 100))
	{
		XMStoreFloat3(Position, DirectX::XMVectorLerp(XMLoadFloat3(Position), XMLoadFloat3(&newPosition), 0.1f));
		HasTransformUpdateRequest = true;
	}

	ImGui::Separator();
	if (ImGui::SliderFloat3("Rotation", &Rotation->x, -180, 180))
	{
		HasTransformUpdateRequest = true;
	}

	ImGui::Separator();
	DirectX::XMFLOAT3 newScale = *Scale;
	if (ImGui::SliderFloat3("Scale", &newScale.x, 0.1f, 10))
	{
		XMStoreFloat3(Scale, DirectX::XMVectorLerp(XMLoadFloat3(Scale), XMLoadFloat3(&newScale), 0.1f));

		HasTransformUpdateRequest = true;
	}
}