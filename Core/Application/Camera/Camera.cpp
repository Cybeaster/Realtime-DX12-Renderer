
#include "Camera.h"

#include "Animations/Animations.h"
#include "Engine/Engine.h"
#include "Window/Window.h"

using namespace DirectX;

OCamera::OCamera(OWindow* _Window)
    : Window(_Window)
{
}

OCamera::OCamera()
{
}

DirectX::XMVECTOR OCamera::GetPosition() const
{
	return DirectX::XMLoadFloat3(&Position);
}

DirectX::XMFLOAT3 OCamera::GetPosition3f() const
{
	return Position;
}

void OCamera::SetPosition(float x, float y, float z)
{
	Position = DirectX::XMFLOAT3(x, y, z);
}

void OCamera::SetPosition(const DirectX::XMFLOAT3& v)
{
	Position = v;
}

DirectX::XMVECTOR OCamera::GetRight() const
{
	return DirectX::XMLoadFloat3(&Right);
}

DirectX::XMFLOAT3 OCamera::GetRight3f() const
{
	return Right;
}

float OCamera::GetNearWindowWidth() const
{
	return Aspect * NearWindowHeight;
}

float OCamera::GetNearWindowHeight() const
{
	return NearWindowHeight;
}

float OCamera::GetFarWindowWidth() const
{
	return Aspect * FarWindowHeight;
}

float OCamera::GetFarWindowHeight() const
{
	return FarWindowHeight;
}

void OCamera::LookAt(DirectX::FXMVECTOR Pos, DirectX::FXMVECTOR _Target, DirectX::FXMVECTOR WorldUp)
{
	const XMVECTOR L = XMVector3Normalize(XMVectorSubtract(_Target, Pos));
	const XMVECTOR R = XMVector3Normalize(XMVector3Cross(WorldUp, L));
	const XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&Position, Pos);
	XMStoreFloat3(&Target, L);
	XMStoreFloat3(&Right, R);
	XMStoreFloat3(&Up, U);

	bViewDirty = true;
}

void OCamera::LookAt(const DirectX::XMFLOAT3& Pos, const DirectX::XMFLOAT3& Target, const DirectX::XMFLOAT3& Up)
{
	XMVECTOR P = XMLoadFloat3(&Pos);
	XMVECTOR T = XMLoadFloat3(&Target);
	XMVECTOR U = XMLoadFloat3(&Up);

	LookAt(P, T, U);

	bViewDirty = true;
}

DirectX::XMFLOAT4X4 OCamera::GetView4x4f() const
{
	return ViewMatrix;
}

DirectX::XMMATRIX OCamera::GetView() const
{
	return DirectX::XMLoadFloat4x4(&ViewMatrix);
}

DirectX::XMFLOAT4X4 OCamera::GetProj4x4f() const
{
	return ProjectionMatrix;
}

DirectX::XMMATRIX OCamera::GetProj() const
{
	return XMLoadFloat4x4(&ProjectionMatrix);
}

void OCamera::SetLens(float _FovY, float _Aspect, float Zn, float Zf)
{
	FovY = _FovY;
	Aspect = _Aspect;
	NearZ = Zn;
	FarZ = Zf;

	NearWindowHeight = 2.0f * NearZ * tanf(0.5f * FovY);
	const DirectX::XMMATRIX projection = MatrixPerspective(FovY, Aspect, NearZ, FarZ);

	DirectX::XMStoreFloat4x4(&ProjectionMatrix, projection);
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, projection);
	Frustum.ConstructFromGeometry(frustum);
	UpdateViewMatrix();
}
void OCamera::SetNewFar(float NewFar)
{
	if (FarZ - NewFar > EPSILON)
	{
		FarZ = NewFar;
		SetLens(FovY, Aspect, NearZ, FarZ);
		MaxCameraSpeed = FarZ;
	}
}

void OCamera::Strafe(float D)
{
	//Position += D * Right;
	XMVECTOR s = XMVectorReplicate(D * CameraSpeed);
	XMVECTOR r = XMLoadFloat3(&Right);
	XMVECTOR p = XMLoadFloat3(&Position);
	XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, r, p));
	bViewDirty = true;
}

void OCamera::MoveToTarget(float D)
{
	//Position += D * Target;
	XMVECTOR s = XMVectorReplicate(D * CameraSpeed);
	XMVECTOR p = XMLoadFloat3(&Position);
	XMVECTOR l = XMLoadFloat3(&Target);
	XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, l, p));
	bViewDirty = true;
}

void OCamera::Pitch(float Angle)
{
	// Rotate up and look vector about the right vector.
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&Right), Angle * CameraSensivity);
	XMVECTOR upVec = XMVector3TransformNormal(XMLoadFloat3(&Up), R);
	XMVECTOR targetVec = XMVector3TransformNormal(XMLoadFloat3(&Target), R);

	// Normalize and orthogonalize
	targetVec = XMVector3Normalize(targetVec);
	upVec = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&Right), targetVec));
	XMStoreFloat3(&Up, upVec);
	XMStoreFloat3(&Target, targetVec);
	bViewDirty = true;
}

void OCamera::RotateY(float Angle)
{
	// Rotate the basis vectors about the world y-axis.
	const XMMATRIX R = XMMatrixRotationY(Angle * CameraSensivity);
	XMVECTOR rightVec = XMVector3TransformNormal(XMLoadFloat3(&Right), R);
	XMVECTOR upVec = XMVector3TransformNormal(XMLoadFloat3(&Up), R);
	XMVECTOR targetVec = XMVector3TransformNormal(XMLoadFloat3(&Target), R);

	// Normalize and orthogonalize
	targetVec = XMVector3Normalize(targetVec);
	rightVec = XMVector3Normalize(rightVec);
	upVec = XMVector3Normalize(XMVector3Cross(targetVec, rightVec));

	XMStoreFloat3(&Right, rightVec);
	XMStoreFloat3(&Up, upVec);
	XMStoreFloat3(&Target, targetVec);
	bViewDirty = true;
}

void OCamera::UpdateViewMatrix()
{
	PerformCameraAnimation(OEngine::Get()->GetDeltaTime());
	if (bViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&Right);
		XMVECTOR U = XMLoadFloat3(&Up);
		XMVECTOR L = XMLoadFloat3(&Target);
		XMVECTOR P = XMLoadFloat3(&Position);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		R = XMVector3Cross(U, L);

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&Right, R);
		XMStoreFloat3(&Up, U);
		XMStoreFloat3(&Target, L);

		ViewMatrix(0, 0) = Right.x;
		ViewMatrix(1, 0) = Right.y;
		ViewMatrix(2, 0) = Right.z;
		ViewMatrix(3, 0) = x;

		ViewMatrix(0, 1) = Up.x;
		ViewMatrix(1, 1) = Up.y;
		ViewMatrix(2, 1) = Up.z;
		ViewMatrix(3, 1) = y;

		ViewMatrix(0, 2) = Target.x;
		ViewMatrix(1, 2) = Target.y;
		ViewMatrix(2, 2) = Target.z;
		ViewMatrix(3, 2) = z;

		ViewMatrix(0, 3) = 0.0f;
		ViewMatrix(1, 3) = 0.0f;
		ViewMatrix(2, 3) = 0.0f;
		ViewMatrix(3, 3) = 1.0f;

		bViewDirty = false;
		OnCameraUpdate.Broadcast();
	}
}
void OCamera::UpdateCameraSpeed(float Delta)
{
	CameraSpeed += Delta * MaxCameraSpeed * 0.01;
	CameraSpeed = std::clamp(CameraSpeed, 0.f, MaxCameraSpeed);
}

void OCamera::SetCameraAnimation(const shared_ptr<OAnimation>& InAnimation)
{
	XMMATRIX rotationMatrix(
	    XMLoadFloat3(&Right),
	    XMLoadFloat3(&Up),
	    XMLoadFloat3(&Target),
	    XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));

	STransform current;
	current.Position = Load(Position);
	current.Rotation = XMQuaternionRotationMatrix(rotationMatrix);
	current.Scale = Load(XMFLOAT3{ 1, 1, 1 });
	InAnimation->StartAnimation(current);
	Animation = InAnimation;
}

void OCamera::PerformCameraAnimation(const float Delta)
{
	if (!Animation.expired() && !Animation.lock()->IsFinished() && Animation.lock()->IsPlaying())
	{
		bViewDirty = true;
		const auto [position, rotation, scale] = Animation.lock()->PerfomAnimation(Delta);
		Put(Position, position);
		LOG(Camera, Log, "New Animation camera position: {}", TEXT(Position));
		const XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotation);
		XMStoreFloat3(&Right, Normalize(rotationMatrix.r[0]));
		XMStoreFloat3(&Up, Normalize(rotationMatrix.r[1]));
		XMStoreFloat3(&Target, Normalize(rotationMatrix.r[2]));
	}
}

void OCamera::SetCameraSpeed(const float Speed)
{
	CameraSpeed = std::max(Speed, 0.1f);
}

void OCamera::SetCameraSensivity(float Sensetivity)
{
	CameraSensivity = Sensetivity;
}

float OCamera::GetCameraSpeed() const
{
	return CameraSpeed;
}

float OCamera::GetCameraSensivity() const
{
	return CameraSensivity;
}

std::tuple<XMVECTOR /*ray_origin*/, XMVECTOR /*ray dir*/, XMMATRIX /*invView*/> OCamera::Pick(int32_t Sx, int32_t Sy) const
{
	XMFLOAT4X4 proj = GetProj4x4f();

	float vx = (+2.0f * Sx / Window->GetWidth() - 1.0f) / proj(0, 0);
	float vy = (-2.0f * Sy / Window->GetHeight() + 1.0f) / proj(1, 1);

	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);
	const XMMATRIX view = GetView();
	auto det = XMMatrixDeterminant(view);
	XMMATRIX invView = XMMatrixInverse(&det, view);
	return { rayOrigin, rayDir, invView };
}
void OCamera::FillPassConstant(HLSL::CameraCBuffer& OutOther) const
{
	const XMMATRIX viewProj = XMMatrixMultiply(GetView(), GetProj());
	const XMMATRIX invView = Inverse(GetView());
	const XMMATRIX invProj = Inverse(GetProj());
	const XMMATRIX invViewProj = Inverse(viewProj);

	Put(OutOther.View, Transpose(GetView()));
	Put(OutOther.InvView, Transpose(invView));
	Put(OutOther.Proj, Transpose(GetProj()));
	Put(OutOther.InvProj, Transpose(invProj));
	Put(OutOther.ViewProj, Transpose(viewProj));
	Put(OutOther.InvViewProj, Transpose(invViewProj));
	OutOther.EyePosW = GetPosition3f();
}

void OCamera::StopAnimation() const
{
	if (!Animation.expired())
	{
		Animation.lock()->StopAnimation();
	}
}

void OCamera::PauseAnimation() const
{
	if (!Animation.expired())
	{
		Animation.lock()->PauseAnimation();
	}
}

float OCamera::GetAperture() const
{
	return Aperture;
}

float OCamera::GetFocusDistance() const
{
	return FocusDistance;
}

DirectX::XMVECTOR OCamera::GetUp() const
{
	return XMLoadFloat3(&Up);
}

DirectX::XMFLOAT3 OCamera::GetUp3f() const
{
	return Up;
}

DirectX::XMVECTOR OCamera::GetLook() const
{
	return XMLoadFloat3(&Target);
}

DirectX::XMFLOAT3 OCamera::GetLook3f() const
{
	return Target;
}

DirectX::XMFLOAT3 OCamera::GetRotation3fEulerAngles() const
{
	auto converter = 180 / XM_PI;
	return GetRotation3fEulerRadians() * converter;
}

DirectX::XMFLOAT3 OCamera::GetRotation3fEulerRadians() const
{
	// Normalize the input vectors
	XMFLOAT3 normTarget = Normalize(Target);
	XMFLOAT3 normUp = Normalize(Up);

	// Calculate Yaw
	const auto yaw = atan2(normTarget.x, normTarget.z);

	// Calculate Pitch
	float targetLengthXZ = sqrt(normTarget.x * normTarget.x + normTarget.z * normTarget.z);
	const auto pitch = atan2(normTarget.y, targetLengthXZ);
	const auto roll = atan2(-normUp.x, normUp.y);
	return { pitch, yaw, roll };
}

float OCamera::GetNearZ() const
{
	return NearZ;
}

float OCamera::GetFarZ() const
{
	return FarZ;
}

float OCamera::GetAspect() const
{
	return Aspect;
}

float OCamera::GetFovY() const
{
	return FovY;
}

float OCamera::GetFovX() const
{
	const float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / GetNearZ());
}