//
// Created by Cybea on 02/02/2024.
//

#include "GeometryEntityWidget.h"

#include "DXHelper.h"
#include "Engine/Engine.h"
#include "UI/Geometry/GeometryManager.h"
void OGeometryEntityWidget::Draw()
{
	const ImVec2 submeshWindowPos = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y);
	ImGui::SetNextWindowPos(submeshWindowPos);
	ImGui::SetNextWindowSize(ImVec2(SliderWidth, 500));

	if (ImGui::Begin("Submesh parameters")) // Begin new window for Submesh parameters
	{
		OHierarchicalWidgetBase::Draw();

		ImGui::SeparatorText("Submeshes");
		if (ImGui::BeginListBox("##Submeshes"))
		{
			for (auto& Submesh : GetGeometry()->DrawArgs)
			{
				auto name = Submesh.first.c_str();

				if (ImGui::Selectable(name, SelectedSubmesh == name))
				{
					SelectedSubmesh = name;
				}
			}
			ImGui::EndListBox();
		}

		if (SelectedSubmesh != "" && GetGeometry()->DrawArgs.contains(SelectedSubmesh))
		{
			const auto& Submesh = GetGeometry()->DrawArgs[SelectedSubmesh];
			const auto formattedName = "Submesh " + SelectedSubmesh;
			ImGui::SeparatorText(formattedName.c_str());
			if (Submesh.Vertices)
			{
				ImGui::SetNextItemWidth(SliderWidth - 50);
				ImGui::Text("Vertices & Indices");
				if (ImGui::BeginListBox("##Vertices & Indices"))
				{
					int32_t counter = 0;
					for (auto& vertex : *Submesh.Vertices)
					{
						string vertexName = "##Vertex: " + std::to_string(counter);
						DirectX::XMFLOAT3 newVertex = vertex;
						if (ImGui::SliderFloat3(vertexName.c_str(), &newVertex.x, -100.0f, 100.0f))
						{
							DirectX::XMStoreFloat3(&vertex, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&vertex), DirectX::XMLoadFloat3(&newVertex), 0.1f));
							RebuildRequest();
						}

						ImGui::SameLine();

						string idxName = "##Index: " + std::to_string(counter);
						int32_t idx = counter;
						if (ImGui::InputInt(idxName.c_str(), &idx))
						{
							Submesh.Indices->at(counter) = idx;
							RebuildRequest();
						}

						counter++;
					}
					ImGui::EndListBox();
				}
			}
		}
	}
	ImGui::End(); // End Submesh parameters window
}

void OGeometryEntityWidget::Update()
{
	using namespace DirectX;
	OHierarchicalWidgetBase::Update();

	if (TransformWidget->SatisfyUpdateRequest())
	{
		// Convert to XMVECTOR for operations
		const XMVECTOR vPosition = XMLoadFloat3(&Position);
		const XMVECTOR vRotation = XMLoadFloat3(&Rotation);
		const XMVECTOR vScale = XMLoadFloat3(&Scale);

		// Create individual transform matrices
		const XMMATRIX matScale = XMMatrixScalingFromVector(vScale);
		const XMMATRIX matRotation = XMMatrixRotationRollPitchYawFromVector(vRotation);
		const XMMATRIX matTranslation = XMMatrixTranslationFromVector(vPosition);

		// Combine into a single transform matrix
		const XMMATRIX transformMatrix = matScale * matRotation * matTranslation;
		RenderItem->UpdateWorldMatrix(transformMatrix);
	}
}

void OGeometryEntityWidget::Init()
{
	IWidget::Init();
	// Variables to store the decomposed components
	DirectX::XMVECTOR scale, rotation, translation;
	if (DirectX::XMMatrixDecompose(&scale, &rotation, &translation, DirectX::XMLoadFloat4x4(&RenderItem->World)))
	{
		// If the decompose fails, set the default values
		Position = { 0, 0, 0 };
		Rotation = { 0, 0, 0 };
		Scale = { 1, 1, 1 };
	}

	DirectX::XMStoreFloat3(&Position, translation);
	DirectX::XMStoreFloat3(&Rotation, rotation);
	DirectX::XMStoreFloat3(&Scale, scale);

	TransformWidget = MakeWidget<OGeometryTransformWidget>(&Position, &Rotation, &Scale);
}

void OGeometryEntityWidget::RebuildRequest()
{
	Manager->RebuildRequest();
}
