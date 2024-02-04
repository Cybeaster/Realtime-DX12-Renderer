#include "Camera.h"

#include "Camera/Camera.h"

OCameraWidget::OCameraWidget(const shared_ptr<OCamera>& Other)
    : Camera(Other)
{
	View = GetCamera()->GetView4x4f();
	CameraSpeed = GetCamera()->GetCameraSpeed();
	CameraSensivity = GetCamera()->GetCameraSensivity();
}

void OCameraWidget::Draw()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		if (ImGui::TreeNode("Camera Speed & Sensivity"))
		{
			if (ImGui::SliderFloat("Speed", &CameraSpeed, 50.f, GetCamera()->MaxCameraSpeed))
			{
				bUpdateSpeed = true;
			}
			if (ImGui::SliderFloat("Sensivity", &CameraSensivity, 0.01f, 5.f))
			{
				bUpdateSensitivy = true;
			}
			ImGui::TreePop();
		}

		ImGui::Text("Camera Position: %f, %f, %f", GetCamera()->GetPosition3f().x, GetCamera()->GetPosition3f().y, GetCamera()->GetPosition3f().z);
		ImGui::Text("Camera Right: %f, %f, %f", GetCamera()->GetRight3f().x, GetCamera()->GetRight3f().y, GetCamera()->GetRight3f().z);
		ImGui::Text("Camera Up: %f, %f, %f", GetCamera()->GetUp3f().x, GetCamera()->GetUp3f().y, GetCamera()->GetUp3f().z);
		ImGui::Text("Camera Look: %f, %f, %f", GetCamera()->GetLook3f().x, GetCamera()->GetLook3f().y, GetCamera()->GetLook3f().z);
	}
}

void OCameraWidget::Update()
{
	IWidget::Update();
	if (Camera.expired())
	{
		return;
	}

	CameraSensivity = bUpdateSensitivy ? CameraSensivity : GetCamera()->GetCameraSensivity();
	CameraSpeed = bUpdateSpeed ? CameraSpeed : GetCamera()->GetCameraSpeed();
	GetCamera()->SetCameraSensivity(CameraSensivity);
	GetCamera()->SetCameraSpeed(CameraSpeed);
	bUpdateSensitivy = false;
	bUpdateSpeed = false;
}