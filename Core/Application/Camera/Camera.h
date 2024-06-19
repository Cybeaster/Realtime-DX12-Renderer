#pragma once

#include "DirectX/BoundingGeometry.h"
#include "Events.h"

#include <DirectXMath.h>
#include <directxcollision.h>

#include <tuple>

class OAnimation;
class OWindow;
class OCamera
{
	DECLARE_DELEGATE(FOnCameraUpdate);

public:
	OCamera(OWindow* _Window);
	OCamera();
	float MaxCameraSpeed = 150;

	// Get/Set world camera position.
	DirectX::XMVECTOR GetPosition() const;
	DirectX::XMFLOAT3 GetPosition3f() const;

	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);

	// Get camera basis vectors.
	DirectX::XMVECTOR GetRight() const;
	DirectX::XMFLOAT3 GetRight3f() const;

	DirectX::XMVECTOR GetUp() const;
	DirectX::XMFLOAT3 GetUp3f() const;
	DirectX::XMVECTOR GetLook() const;
	DirectX::XMFLOAT3 GetLook3f() const;
	DirectX::XMFLOAT3 GetRotation3fEulerAngles() const;
	DirectX::XMFLOAT3 GetRotation3fEulerRadians() const;

	// Get frustum properties.
	float GetNearZ() const;
	float GetFarZ() const;
	float GetAspect() const;
	float GetFovY() const;
	float GetFovX() const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth() const;
	float GetNearWindowHeight() const;
	float GetFarWindowWidth() const;
	float GetFarWindowHeight() const;

	void LookAt(DirectX::FXMVECTOR Pos, DirectX::FXMVECTOR Target, DirectX::FXMVECTOR Up);
	void LookAt(const DirectX::XMFLOAT3& Pos, const DirectX::XMFLOAT3& Target, const DirectX::XMFLOAT3& Up);

	DirectX::XMFLOAT4X4 GetView4x4f() const;
	DirectX::XMMATRIX GetView() const;

	DirectX::XMFLOAT4X4 GetProj4x4f() const;
	DirectX::XMMATRIX GetProj() const;
	// Set frustum.
	void SetLens(float FovY, float Aspect, float Zn, float Zf);
	void SetNewFar(float NewFar);
	void Strafe(float D);
	void MoveToTarget(float D);

	void Pitch(float Angle);
	void RotateY(float Angle);

	void UpdateViewMatrix();
	void UpdateCameraSpeed(float Delta);
	void SetCameraAnimation(const shared_ptr<OAnimation>& InAnimation);
	void SetCameraSpeed(float Speed);
	void SetCameraSensivity(float Sensetivity);
	float GetCameraSpeed() const;
	float GetCameraSensivity() const;
	OBoundingFrustum& GetFrustum() { return Frustum; };

	std::tuple<DirectX::XMVECTOR /*ray_origin*/, DirectX::XMVECTOR /*ray dir*/, DirectX::XMMATRIX /*invView*/> Pick(int32_t Sx, int32_t Sy) const;
	void FillPassConstant(HLSL::CameraCBuffer& OutOther) const;
	void StopAnimation() const;
	void PauseAnimation() const;
	FOnCameraUpdate OnCameraUpdate;
	float GetAperture() const;
	float GetFocusDistance() const;
private:
	void PerformCameraAnimation(float Delta);
	weak_ptr<OAnimation> Animation;
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Target = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 Up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 Right = { 1.0, 0.0, 1.0 };

	float CameraSpeed = 400.f;
	float CameraSensivity = 0.5f;

	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float Aspect = 0.0f;
	float FovY = 0.0f;
	float NearWindowHeight = 0.0f;
	float FarWindowHeight = 0.0f;
	float Aperture = 0.01f;
	float FocusDistance = 10.0f;
	DirectX::XMFLOAT4X4 ViewMatrix;
	DirectX::XMFLOAT4X4 ProjectionMatrix;

	bool bViewDirty = true;
	OWindow* Window;

	OBoundingFrustum Frustum;
};
