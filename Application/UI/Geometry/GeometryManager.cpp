#include "GeometryManager.h"

void OGeometryManagerWidget::Draw()
{
	CHECK(GeometryMap != nullptr);

	ImGui::Text("Geometry manager");
	ImGui::SameLine();
	ImGui::Text("Number of geometries %d", GeometryMap->size());


}

void OGeometryManagerWidget::Update()
{
	IWidget::Update();
}