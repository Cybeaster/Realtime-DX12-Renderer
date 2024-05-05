#pragma once
#include "UI/Widget.h"

#include <DirectXMath.h>

class OCamera;
class OCameraWidget : public IWidget
{
public:
	OCameraWidget(const shared_ptr<OCamera>& Other);
	;

	shared_ptr<OCamera> GetCamera() const
	{
		return Camera.lock();
	}

	void Draw() override;
	void Update() override;

private:
	bool bUpdateSpeed = false;
	bool bUpdateSensitivy = false;
	float CameraSpeed;
	float CameraSensivity;
	DirectX::XMFLOAT4X4 View;

	weak_ptr<OCamera> Camera;
};