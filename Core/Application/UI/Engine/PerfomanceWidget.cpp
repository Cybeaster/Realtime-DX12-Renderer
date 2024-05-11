
#include "PerfomanceWidget.h"

#include "Engine/Engine.h"
void OPerfomanceWidget::Draw()
{
	if (ImGui::CollapsingHeader("Perfomance Info"))
	{
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Number of rendered meshes: %d", OEngine::Get()->GetRenderedItems().Items.size());
		size_t numTriangles = 0;
		for (const auto& item : OEngine::Get()->GetRenderedItems().Items)
		{
			numTriangles += item.first.lock()->ChosenSubmesh.lock()->Vertices->size() / 3;
		}
		ImGui::Text("Number of rendered triangles: %d", numTriangles);
		ImGui::Checkbox("Enable Frustum Cooling", &OEngine::Get()->bFrustrumCullingEnabled);
		ImGui::Checkbox("Enable Logs", &SLogUtils::bLogToConsole);
	}
}