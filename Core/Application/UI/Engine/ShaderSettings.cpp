#include "ShaderSettings.h"

#include "Engine/Engine.h"

void OShaderSettings::Draw()
{
	if (ImGui::CollapsingHeader("Shader Settings"))
	{
		ImGui::Text("Shader Settings");
		ImGui::Separator();
		if (ImGui::Button("Reload Shaders"))
		{
			OEngine::Get()->ReloadShaders();
		}
	}
}