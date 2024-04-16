
#include "PerfomanceWidget.h"

#include "Engine/Engine.h"
void OPerfomanceWidget::Draw()
{
	if (ImGui::CollapsingHeader("Perfomance Info"))
	{
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Number of rendered meshes: %d", OEngine::Get()->GetRenderedItems().size());
		size_t numTriangles = 0;
		for (const auto& item : OEngine::Get()->GetRenderedItems())
		{
			numTriangles += item->ChosenSubmesh->Vertices->size() / 3;
		}
		ImGui::Text("Number of rendered triangles: %d", numTriangles);
		ImGui::Checkbox("Enable Frustum Cooling", &OEngine::Get()->bFrustrumCullingEnabled);
	}
}