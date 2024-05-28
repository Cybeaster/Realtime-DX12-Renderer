#pragma once
#include "DirectX/DXHelper.h"

#include <DirectXMath.h>
#include <corecrt_math.h>

#include <random>
#define LEFT_HANDED true
constexpr float Infinity = FLT_MAX;
constexpr float EPSILON = 1e-6f;

namespace Utils::Math
{
using namespace DirectX;

template<typename T>
T Clamp(const T& x, const T& Low, const T& High)
{
	return x < Low ? Low : (x > High ? High : x);
}

//clang-format off
inline DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
	    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}
//clang-format on

inline auto Abs(const DirectX::XMVECTOR& Vec)
{
	return DirectX::XMVectorAbs(Vec);
}

template<typename Type>
inline auto Lerp(float T, const Type& A, const Type& B)
{
	return (1.0f - T) * A + T * B; // The interpolated vector
}

template<typename Type>
inline auto IsNormalized(const Type& A, float EPSILON = 1e-6f)
{
	return abs(1.0f - Length(A)) < EPSILON; // Return true if the vector is normalized
}

template<typename T>
auto Squared(T Value)
{
	return Value * Value;
}

template<typename Type>
inline auto DegreesToRaians(auto Degrees)
{
	return Degrees * (DirectX::XM_PI / 180.0f);
}

template<typename T = double>
inline auto Random()
{
	static std::uniform_real_distribution<T> Distribution(0, 1); // Random double distribution
	static std::mt19937 Generator; // Random number generator
	return Distribution(Generator); // Return a random double between 0 and 1
}

template<typename T = double>
inline auto Random(T Min, T Max)
{
	return Min + (Max - Min) * Random<T>(); // Return a random double between Min and Max
}

inline int32_t Random(int32_t Min, int32_t Max)
{
	return Random<double>(Min, Max + 1);
}

inline DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX Matrix)
{
	DirectX::XMMATRIX A = Matrix;
	A.r[3] = DirectX::XMVectorSet(0.f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR Determinant = DirectX::XMMatrixDeterminant(A);
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&Determinant, A));
}

inline DirectX::XMVECTOR SphericalToCartesian(float Radius, float Theta, float Phi)
{
	return DirectX::XMVectorSet(
	    Radius * cosf(Theta) * sinf(Phi),
	    Radius * cosf(Phi),
	    Radius * sinf(Theta) * sinf(Phi),
	    1.0f);
}

inline void ComputeTangent(
    const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2,
    const XMFLOAT2& uv0, const XMFLOAT2& uv1, const XMFLOAT2& uv2,
    XMFLOAT3& tangent)
{
	XMVECTOR p0 = XMLoadFloat3(&v0);
	XMVECTOR p1 = XMLoadFloat3(&v1);
	XMVECTOR p2 = XMLoadFloat3(&v2);

	XMVECTOR uv0v = XMLoadFloat2(&uv0);
	XMVECTOR uv1v = XMLoadFloat2(&uv1);
	XMVECTOR uv2v = XMLoadFloat2(&uv2);

	// Compute deltas for positions and UVs
	XMVECTOR deltaPos1 = p1 - p0;
	XMVECTOR deltaPos2 = p2 - p0;

	XMFLOAT2 deltaUV1, deltaUV2;
	XMStoreFloat2(&deltaUV1, uv1v - uv0v);
	XMStoreFloat2(&deltaUV2, uv2v - uv0v);

	// Computing the determinant of a 2x2 matrix from UV deltas
	float det = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
	auto eps = 1e-6f;
	float r = det >= eps ? 1.0f / det : 0.0f; // Handle the zero determinant case

	// Calculate tangent using the formula provided
	// T = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * (1.0 / det)
	XMVECTOR tangentv = (deltaPos1 * XMVectorSet(deltaUV2.y, deltaUV2.y, deltaUV2.y, 0.0f) - deltaPos2 * XMVectorSet(deltaUV1.y, deltaUV1.y, deltaUV1.y, 0.0f)) * XMVectorReplicate(r);

	tangentv = XMVector3Normalize(tangentv);

	XMStoreFloat3(&tangent, tangentv);
}
} // namespace Utils::Math

inline DirectX::XMMATRIX Inverse(const DirectX::XMMATRIX& Mat, bool WithDet = true)
{
	if (WithDet)
	{
		auto det = XMMatrixDeterminant(Mat);
		return XMMatrixInverse(&det, Mat);
	}
	else
	{
		return XMMatrixInverse(nullptr, Mat);
	}
}

inline void Inverse(DirectX::XMMATRIX& OutMat)
{
	auto det = XMMatrixDeterminant(OutMat);
	OutMat = XMMatrixInverse(&det, OutMat);
}

inline void Transpose(DirectX::XMMATRIX& OutMat)
{
	OutMat = XMMatrixTranspose(OutMat);
}

inline DirectX::XMMATRIX Transpose(const DirectX::XMMATRIX& Mat)
{
	return XMMatrixTranspose(Mat);
}

inline void Put(DirectX::XMFLOAT4X4& OutOther, const DirectX::XMMATRIX& Mat)
{
	XMStoreFloat4x4(&OutOther, Mat);
}

inline void Put(DirectX::XMFLOAT4& OutOther, const DirectX::XMVECTOR& Vec)
{
	XMStoreFloat4(&OutOther, Vec);
}

inline void Put(DirectX::XMFLOAT3& OutOther, const DirectX::XMVECTOR& Vec)
{
	XMStoreFloat3(&OutOther, Vec);
}

inline void Put(DirectX::XMFLOAT2& OutOther, const DirectX::XMVECTOR& Vec)
{
	XMStoreFloat2(&OutOther, Vec);
}

inline DirectX::XMFLOAT2 operator-(const DirectX::XMFLOAT2& A, const DirectX::XMFLOAT2& B)
{
	return { A.x - B.x, A.y - B.y };
}

inline DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& A, const DirectX::XMFLOAT3& B)
{
	return { A.x - B.x, A.y - B.y, A.z - B.z };
}

inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& A, const DirectX::XMFLOAT3& B)
{
	return { A.x + B.x, A.y + B.y, A.z + B.z };
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& A, const DirectX::XMFLOAT3& B)
{
	return { A.x * B.x, A.y * B.y, A.z * B.z };
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& A, float B)
{
	return { A.x * B, A.y * B, A.z * B };
}

inline DirectX::XMMATRIX Load(const DirectX::XMMATRIX& Source)
{
	return Source;
}

inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4& Source)
{
	return XMLoadFloat4x4(&Source);
}

inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3& Source)
{
	return DirectX::XMLoadFloat3(&Source);
}

inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4& Source)
{
	return DirectX::XMLoadFloat4(&Source);
}

inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT2& Source)
{
	return DirectX::XMLoadFloat2(&Source);
}

inline DirectX::XMFLOAT4X4 Scale(DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT3 ScaleFactor)
{
	Put(OutOther, DirectX::XMMatrixMultiply(Load(OutOther), DirectX::XMMatrixScaling(ScaleFactor.x, ScaleFactor.y, ScaleFactor.z)));
	return OutOther;
}

inline DirectX::XMFLOAT4X4 Scale(const DirectX::XMFLOAT4X4& Other, DirectX::XMFLOAT3 ScaleFactor)
{
	auto other = Other;
	Put(other, DirectX::XMMatrixMultiply(Load(other), DirectX::XMMatrixScaling(ScaleFactor.x, ScaleFactor.y, ScaleFactor.z)));
	return other;
}

inline DirectX::XMFLOAT4X4 Translate(DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT3 TranslationFactor)
{
	//Translate the matrix by the given factor
	Put(OutOther, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&OutOther), DirectX::XMMatrixTranslation(TranslationFactor.x, TranslationFactor.y, TranslationFactor.z)));
	return OutOther;
}

inline DirectX::XMFLOAT4 Rotate(const DirectX::XMFLOAT4& OutOther, float Angle, DirectX::XMFLOAT3 Axis)
{
	auto other = OutOther;
	Put(other, DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&OutOther), DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&Axis), Angle)));
	return other;
}

inline DirectX::XMFLOAT4X4 Rotate(const DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT3 QuatRotation)
{
	auto other = OutOther;
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(Load(QuatRotation));
	Put(other, DirectX::XMMatrixMultiply(Load(other), RotationMatrix));
	return other;
}

inline DirectX::XMFLOAT4X4 Rotate(const DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT4 QuatRotation)
{
	auto other = OutOther;
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(Load(QuatRotation));
	Put(other, DirectX::XMMatrixMultiply(Load(other), RotationMatrix));
	return other;
}

inline void Rotate(DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT3 QuatRotation)
{
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(Load(QuatRotation));
	Put(OutOther, DirectX::XMMatrixMultiply(Load(OutOther), RotationMatrix));
}

inline void Rotate(DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT4 QuatRotation)
{
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(Load(QuatRotation));
	Put(OutOther, DirectX::XMMatrixMultiply(Load(OutOther), RotationMatrix));
}

inline DirectX::XMFLOAT4X4 Translate(const DirectX::XMFLOAT4X4& Other, DirectX::XMFLOAT3 TranslationFactor)
{
	auto other = Other;
	Put(other, XMMatrixMultiply(Load(other), DirectX::XMMatrixTranslation(TranslationFactor.x, TranslationFactor.y, TranslationFactor.z)));
	return other;
}

inline DirectX::XMFLOAT4X4 MatCast(const DirectX::XMMATRIX& Mat)
{
	DirectX::XMFLOAT4X4 other;
	XMStoreFloat4x4(&other, Mat);
	return other;
}

inline void QuaternionToEulerAngles(const DirectX::XMFLOAT4& Quat, DirectX::XMFLOAT3& Euler)
{
	using namespace DirectX;

	// Roll (x-axis rotation)
	double sinr_cosp = 2.0 * (Quat.w * Quat.x + Quat.y * Quat.z);
	double cosr_cosp = 1.0 - 2.0 * (Quat.x * Quat.x + Quat.y * Quat.y);
	Euler.x = std::atan2(sinr_cosp, cosr_cosp);

	// Pitch (y-axis rotation)
	double sinp = 2.0 * (Quat.w * Quat.y - Quat.z * Quat.x);
	if (std::fabs(sinp) >= 1)
		Euler.y = std::copysign(XM_PI / 2, sinp); // use 90 degrees if out of range
	else
		Euler.y = std::asin(sinp);

	// Yaw (z-axis rotation)
	double siny_cosp = 2.0 * (Quat.w * Quat.z + Quat.x * Quat.y);
	double cosy_cosp = 1.0 - 2.0 * (Quat.y * Quat.y + Quat.z * Quat.z);
	Euler.z = std::atan2(siny_cosp, cosy_cosp);
}

inline void QuaternionToEulerAngles(const DirectX::XMVECTOR& Quat, DirectX::XMFLOAT3& Euler)
{
	using namespace DirectX;
	XMFLOAT4 q;
	XMStoreFloat4(&q, Quat);
	QuaternionToEulerAngles(q, Euler);
}

inline DirectX::XMMATRIX MatrixPerspective(float Fov, float Aspect, float NearZ, float FarZ)
{
	if (LEFT_HANDED)
	{
		return DirectX::XMMatrixPerspectiveFovLH(Fov, Aspect, NearZ, FarZ);
	}
	else
	{
		return DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, NearZ, FarZ);
	}
}

inline auto MatrixLookAt(const DirectX::XMFLOAT3& Eye, const DirectX::XMFLOAT3& Target, const DirectX::XMFLOAT3& Up)
{
	if (LEFT_HANDED)
	{
		return DirectX::XMMatrixLookAtLH(Load(Eye), Load(Target), Load(Up));
	}
	else
	{
		return DirectX::XMMatrixLookAtRH(Load(Eye), Load(Target), Load(Up));
	}
}

inline auto MatrixLookAt(const DirectX::XMVECTOR& Eye, const DirectX::XMVECTOR& Target, const DirectX::XMVECTOR& Up)
{
	if (LEFT_HANDED)
	{
		return DirectX::XMMatrixLookAtLH(Eye, Target, Up);
	}
	else
	{
		return DirectX::XMMatrixLookAtRH(Eye, Target, Up);
	}
}

inline auto MatrixOrthographic(float Width, float Height, float NearZ, float FarZ)
{
	if (LEFT_HANDED)
	{
		return DirectX::XMMatrixOrthographicLH(Width, Height, NearZ, FarZ);
	}
	else
	{
		return DirectX::XMMatrixOrthographicRH(Width, Height, NearZ, FarZ);
	}
}

inline auto MatrixOrthographicOffCenter(float Left, float Right, float Bottom, float Top, float NearZ, float FarZ)
{
	if (LEFT_HANDED)
	{
		return DirectX::XMMatrixOrthographicOffCenterLH(Left, Right, Bottom, Top, NearZ, FarZ);
	}
	else
	{
		return DirectX::XMMatrixOrthographicOffCenterRH(Left, Right, Bottom, Top, NearZ, FarZ);
	}
}

static DirectX::XMFLOAT3 TransformTransposed(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT4X4& matrix)
{
	DirectX::XMFLOAT3 result = {};
	DirectX::XMFLOAT4 temp(point.x, point.y, point.z, 1); //need a 4-part vector in order to multiply by a 4x4 matrix
	DirectX::XMFLOAT4 temp2 = {};

	temp2.x = temp.x * matrix._11 + temp.y * matrix._12 + temp.z * matrix._13 + temp.w * matrix._14;
	temp2.y = temp.x * matrix._21 + temp.y * matrix._22 + temp.z * matrix._23 + temp.w * matrix._24;
	temp2.z = temp.x * matrix._31 + temp.y * matrix._32 + temp.z * matrix._33 + temp.w * matrix._34;
	temp2.w = temp.x * matrix._41 + temp.y * matrix._42 + temp.z * matrix._43 + temp.w * matrix._44;

	result.x = temp2.x / temp2.w; //view projection matrices make use of the W component
	result.y = temp2.y / temp2.w;
	result.z = temp2.z / temp2.w;

	return result;
}

inline DirectX::XMMATRIX BuildWorldMatrix(const DirectX::XMFLOAT3& Position,
                                          const DirectX::XMFLOAT3& Scale,
                                          const DirectX::XMFLOAT4& Quaternion)
{
	using namespace DirectX;
	// Scale matrix
	XMMATRIX scaleMatrix = XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	// Rotation matrix
	XMVECTOR rotationQuat = XMLoadFloat4(&Quaternion);
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);

	// Translation matrix
	XMMATRIX translationMatrix = XMMatrixTranslation(Position.x, Position.y, Position.z);

	// Combine scale, rotation, and translation to form the final world matrix
	return scaleMatrix * rotationMatrix * translationMatrix;
}

inline DirectX::XMMATRIX BuildViewMatrix(const DirectX::XMVECTOR& Origin, const DirectX::XMVECTOR& Orientation, const DirectX::XMVECTOR& Up)
{
	// Compute the forward direction vector from the orientation quaternion
	const DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), Orientation);

	// Compute the target position by adding the forward vector to the origin
	const DirectX::XMVECTOR target = DirectX::XMVectorAdd(Origin, forward);

	// Use XMMatrixLookAtLH to build the view matrix
	return MatrixLookAt(Origin, target, Up);
}

inline DirectX::XMMATRIX BuildOrthographicMatrix(const DirectX::BoundingFrustum& frustum)
{
	const float nearZ = frustum.Near;
	const float farZ = frustum.Far;

	// Calculate the width and height at the near plane
	const float width = frustum.RightSlope * nearZ * 2.0f;
	const float height = frustum.TopSlope * nearZ * 2.0f;

	// Calculate left, right, bottom, top, near, far values
	const float left = -width / 2.0f;
	const float right = width / 2.0f;
	const float bottom = -height / 2.0f;
	const float top = height / 2.0f;

	// Create the orthographic matrix
	return MatrixOrthographicOffCenter(left, right, bottom, top, nearZ, farZ);
}

inline DirectX ::XMFLOAT3 NormalizedToAngles(const DirectX::XMFLOAT3 InRad)
{
	return { InRad.x * 180, InRad.y * 180, InRad.z * 180 };
}

inline DirectX::XMFLOAT3 AnglesToNormalized(const DirectX::XMFLOAT3 InDeg)
{
	return { InDeg.x / 180.f, InDeg.y / 180.f, InDeg.z / 180.f };
}

inline DirectX::XMFLOAT3 NormalizedToAngles(const DirectX::XMVECTOR& InRad)
{
	using namespace DirectX;
	XMFLOAT3 euler;
	XMStoreFloat3(&euler, InRad * 180);
	return euler;
}

inline DirectX::XMVECTOR NormalizedToQuaternion(const DirectX::XMFLOAT3& InRad)
{
	using namespace DirectX;
	DirectX::XMFLOAT3 euler = InRad * XM_PI;
	return XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
}

inline DirectX::XMVECTOR EulerToQuaternion(const DirectX::XMFLOAT3& InRad)
{
	using namespace DirectX;
	return XMQuaternionRotationRollPitchYaw(InRad.x, InRad.y, InRad.z);
}

// Converts a quaternion to a direction vector
inline DirectX::XMFLOAT3 QuaternionToDirection(const DirectX::XMVECTOR& quat)
{
	using namespace DirectX;
	// Default forward vector
	XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR dir = XMVector3Rotate(forward, quat);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, dir);
	return direction;
}

// Converts a quaternion to a direction vector
inline DirectX::XMFLOAT3 QuaternionToDirection(const DirectX::XMFLOAT4& quat)
{
	return QuaternionToDirection(Load(quat));
}

inline DirectX::BoundingOrientedBox TransformBoundingBoxToViewSpace(const DirectX::BoundingOrientedBox& box, const DirectX::XMMATRIX& viewMatrix)
{
	using namespace DirectX;
	// Transform the center of the bounding box to view space
	XMVECTOR center = XMLoadFloat3(&box.Center);
	center = XMVector3Transform(center, viewMatrix);

	// The orientation needs to be transformed as well
	XMVECTOR orientation = XMLoadFloat4(&box.Orientation);
	orientation = XMQuaternionMultiply(orientation, XMQuaternionRotationMatrix(viewMatrix));

	// The extents remain the same since they are axis-aligned with respect to the box's local space
	BoundingOrientedBox viewSpaceBox;
	XMStoreFloat3(&viewSpaceBox.Center, center);
	XMStoreFloat4(&viewSpaceBox.Orientation, orientation);
	viewSpaceBox.Extents = box.Extents;

	return viewSpaceBox;
}

inline bool operator==(const DirectX::XMFLOAT3 A, const DirectX::XMFLOAT3 B)
{
	return abs(A.x - B.x) < EPSILON && abs(A.y - B.y) < EPSILON && abs(A.z - B.z) < EPSILON;
}

inline bool operator==(const DirectX::XMFLOAT2 A, const DirectX::XMFLOAT2 B)
{
	return abs(A.x - B.x) < EPSILON && abs(A.y - B.y) < EPSILON;
}

inline bool operator==(const DirectX::XMFLOAT4 A, const DirectX::XMFLOAT4 B)
{
	return abs(A.x - B.x) < EPSILON && abs(A.y - B.y) < EPSILON && abs(A.z - B.z) < EPSILON && abs(A.w - B.w) < EPSILON;
}

inline bool operator!=(const DirectX::XMFLOAT3 A, const DirectX::XMFLOAT3 B)
{
	return !(operator==(A, B));
}

inline bool operator!=(const DirectX::XMFLOAT2 A, const DirectX::XMFLOAT2 B)
{
	return !(operator==(A, B));
}

inline bool operator!=(const DirectX::XMFLOAT4 A, const DirectX::XMFLOAT4 B)
{
	return !(operator==(A, B));
}

inline auto GetX(const DirectX::XMFLOAT3& Vec)
{
	return Vec.x;
}

inline auto GetY(const DirectX::XMFLOAT3& Vec)
{
	return Vec.y;
}

inline auto GetZ(const DirectX::XMFLOAT3& Vec)
{
	return Vec.z;
}

inline auto GetX(const DirectX::XMFLOAT2& Vec)
{
	return Vec.x;
}

inline auto GetY(const DirectX::XMFLOAT2& Vec)
{
	return Vec.y;
}

inline auto GetX(const DirectX::XMFLOAT4& Vec)
{
	return Vec.x;
}

inline auto GetY(const DirectX::XMFLOAT4& Vec)
{
	return Vec.y;
}

inline auto GetZ(const DirectX::XMFLOAT4& Vec)
{
	return Vec.z;
}

inline auto GetW(const DirectX::XMFLOAT4& Vec)
{
	return Vec.w;
}

inline auto GetW(const DirectX::XMFLOAT3& Vec)
{
	return 1.0f;
}

inline auto GetW(const DirectX::XMFLOAT2& Vec)
{
	return 1.0f;
}

inline auto GetX(const DirectX::XMVECTOR& Vec)
{
	return DirectX::XMVectorGetX(Vec);
}

inline auto GetY(const DirectX::XMVECTOR& Vec)
{
	return DirectX::XMVectorGetY(Vec);
}

inline auto GetZ(const DirectX::XMVECTOR& Vec)
{
	return DirectX::XMVectorGetZ(Vec);
}

inline auto GetW(const DirectX::XMVECTOR& Vec)
{
	return DirectX::XMVectorGetW(Vec);
}
